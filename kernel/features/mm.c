// Hugely inspired by the implementation in skiftOS: MIT License, Copyright (c) 2020 N. Van Bossuyt
// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <cpu.h>
#include <def.h>
#include <errno.h>
#include <fb.h>
#include <mem.h>
#include <mm.h>
#include <print.h>
#include <random.h>

static struct page_dir kernel_dir ALIGNED(PAGE_SIZE) = { 0 };
static struct page_table kernel_tables[PAGE_KERNEL_COUNT] ALIGNED(PAGE_SIZE) = { 0 };

/**
 * Lowlevel paging
 */

static void paging_switch_dir(u32 dir)
{
	cr3_set(dir);
}

extern void paging_invalidate_tlb(void);

/*void paging_disable(void)
{
	cr0_set(cr0_get() | 0x7fffffff);
}*/

void paging_enable(void)
{
	cr0_set(cr0_get() | 0x80000000);
}

void page_fault_handler(struct regs *r)
{
	print("--- PAGE FAULT! ---\n");

	// Check error code
	const char *type = (r->err_code & 1) ? "present" : "non-present";
	const char *operation = (r->err_code & 2) ? "write" : "read";
	const char *super = (r->err_code & 4) ? "User" : "Super";

	// Check cr2 address
	u32 vaddr;
	__asm__ volatile("movl %%cr2, %%eax" : "=a"(vaddr));
	struct proc *proc = proc_current();
	struct page_dir *dir = NULL;
	if (proc && proc->page_dir) {
		dir = proc->page_dir;
		/* printf("Stack is at %x, entry at %x\n", virtual_to_physical(dir, proc->regs.ebp), */
		/*        virtual_to_physical(dir, proc->entry)); */
	} else {
		dir = &kernel_dir;
	}
	u32 paddr = virtual_to_physical(dir, vaddr);

	// Print!
	printf("%s process tried to %s a %s page at [vaddr=%x; paddr=%x]\n", super, operation, type,
	       vaddr, paddr);

	isr_panic(r);
}

/**
 * Physical
 */

static u32 memory_used = 0;
static u32 memory_total = 0;
static u32 best_bet = 0;
static u8 memory[PAGE_COUNT * PAGE_COUNT / 8] = { 0 };

static u8 physical_page_is_used(u32 addr)
{
	u32 page = addr / PAGE_SIZE;
	return memory[page / 8] & (1 << (page % 8));
}

static void physical_page_set_used(u32 address)
{
	u32 page = address / PAGE_SIZE;

	if (page == best_bet)
		best_bet++;

	memory[page / 8] |= 1 << (page % 8);
}

static void physical_page_set_free(u32 address)
{
	u32 page = address / PAGE_SIZE;

	if (page < best_bet)
		best_bet = page;

	memory[page / 8] &= ~(1 << (page % 8));
}

static void physical_set_used(struct memory_range range)
{
	assert(PAGE_ALIGNED(range.base) && PAGE_ALIGNED(range.size));

	for (u32 i = 0; i < range.size / PAGE_SIZE; i++) {
		u32 addr = range.base + i * PAGE_SIZE;
		if (!physical_page_is_used(addr)) {
			memory_used += PAGE_SIZE;
			physical_page_set_used(addr);
		}
	}
}

static void physical_set_free(struct memory_range range)
{
	assert(PAGE_ALIGNED(range.base) && PAGE_ALIGNED(range.size));

	for (u32 i = 0; i < range.size / PAGE_SIZE; i++) {
		u32 addr = range.base + i * PAGE_SIZE;

		if (physical_page_is_used(addr)) {
			memory_used -= PAGE_SIZE;
			physical_page_set_free(addr);
		}
	}
}

static u8 physical_is_used(struct memory_range range)
{
	assert(PAGE_ALIGNED(range.base) && PAGE_ALIGNED(range.size));

	for (u32 i = 0; i < range.size / PAGE_SIZE; i++) {
		u32 addr = range.base + i * PAGE_SIZE;

		if (physical_page_is_used(addr))
			return 1;
	}

	return 0;
}

struct memory_range physical_alloc(u32 size)
{
	assert(PAGE_ALIGNED(size));

	for (u32 i = best_bet; i < ((memory_total - size) / PAGE_SIZE); i++) {
		struct memory_range range = memory_range(i * PAGE_SIZE, size);

		if (!physical_is_used(range)) {
			physical_set_used(range);
			return range;
		}
	}

	panic("Out of physical memory!\n");
	return memory_range(0, 0);
}

void physical_free(struct memory_range range)
{
	assert(PAGE_ALIGNED(range.base) && PAGE_ALIGNED(range.size));
	physical_set_free(range);
}

/**
 * Virtual
 */

#define PDI(vaddr) ((vaddr) >> 22)
#define PTI(vaddr) (((vaddr) >> 12) & 0x03ff)

u8 virtual_present(struct page_dir *dir, u32 vaddr)
{
	u32 pdi = PDI(vaddr);
	union page_dir_entry *dir_entry = &dir->entries[pdi];
	if (!dir_entry->bits.present)
		return 0;

	struct page_table *table = (struct page_table *)(dir_entry->bits.address * PAGE_SIZE);

	u32 pti = PTI(vaddr);
	union page_table_entry *table_entry = &table->entries[pti];

	return table_entry->bits.present;
}

u32 virtual_to_physical(struct page_dir *dir, u32 vaddr)
{
	u32 pdi = PDI(vaddr);
	union page_dir_entry *dir_entry = &dir->entries[pdi];
	if (!dir_entry->bits.present)
		return 0;

	struct page_table *table = (struct page_table *)(dir_entry->bits.address * PAGE_SIZE);

	u32 pti = PTI(vaddr);
	union page_table_entry *table_entry = &table->entries[pti];
	if (!table_entry->bits.present)
		return 0;

	return (table_entry->bits.address * PAGE_SIZE) + (vaddr & (PAGE_SIZE - 1));
}

void virtual_map(struct page_dir *dir, struct memory_range prange, u32 vaddr, u32 flags)
{
	for (u32 i = 0; i < prange.size / PAGE_SIZE; i++) {
		u32 offset = i * PAGE_SIZE;

		u32 pdi = PDI(vaddr + offset);
		union page_dir_entry *dir_entry = &dir->entries[pdi];
		struct page_table *table =
			(struct page_table *)(dir_entry->bits.address * PAGE_SIZE);

		if (!dir_entry->bits.present) {
			table = memory_alloc_identity(dir, MEMORY_CLEAR);
			dir_entry->bits.present = 1;
			dir_entry->bits.writable = 1;
			dir_entry->bits.user = 1;
			dir_entry->bits.address = (u32)(table) >> 12;
		}

		u32 pti = PTI(vaddr + offset);
		union page_table_entry *table_entry = &table->entries[pti];
		table_entry->bits.present = 1;
		table_entry->bits.writable = 1;
		table_entry->bits.user = flags & MEMORY_USER;
		table_entry->bits.address = (prange.base + offset) >> 12;
	}

	paging_invalidate_tlb();
}

struct memory_range virtual_alloc(struct page_dir *dir, struct memory_range prange, u32 flags)
{
	u8 user = flags & MEMORY_USER;
	u32 vaddr = 0;
	u32 size = 0;

	for (u32 i = (user ? PAGE_KERNEL_COUNT : 1) * PAGE_COUNT;
	     i < (user ? PAGE_COUNT : PAGE_KERNEL_COUNT) * PAGE_COUNT; i++) {
		u32 addr = i * PAGE_SIZE;
		if (!virtual_present(dir, addr)) {
			if (size == 0)
				vaddr = addr;

			size += PAGE_SIZE;

			if (size == prange.size) {
				virtual_map(dir, prange, vaddr, flags);
				return memory_range(vaddr, size);
			}
		} else {
			size = 0;
		}
	}

	panic("Out of virtual memory!\n");
	return memory_range(0, 0);
}

void virtual_free(struct page_dir *dir, struct memory_range vrange)
{
	for (u32 i = 0; i < vrange.size / PAGE_SIZE; i++) {
		u32 offset = i * PAGE_SIZE;

		u32 pdi = PDI(vrange.base + offset);
		union page_dir_entry *dir_entry = &dir->entries[pdi];
		if (!dir_entry->bits.present)
			continue;

		struct page_table *table =
			(struct page_table *)(dir_entry->bits.address * PAGE_SIZE);

		u32 pti = PTI(vrange.base + offset);
		union page_table_entry *table_entry = &table->entries[pti];

		if (table_entry->bits.present)
			table_entry->uint = 0;
	}

	paging_invalidate_tlb();
}

struct page_dir *virtual_create_dir(void)
{
	struct page_dir *dir = memory_alloc(&kernel_dir, sizeof(*dir), MEMORY_CLEAR);

	memset(dir, 0, sizeof(*dir));

	for (u32 i = 0; i < PAGE_KERNEL_COUNT; i++) {
		union page_dir_entry *dir_entry = &dir->entries[i];

		dir_entry->bits.present = 1;
		dir_entry->bits.writable = 1;
		dir_entry->bits.user = 0;
		dir_entry->bits.address = (u32)&kernel_tables[i] / PAGE_SIZE;
	}

	return dir;
}

void virtual_destroy_dir(struct page_dir *dir)
{
	assert(dir != &kernel_dir);

	for (u32 i = PAGE_KERNEL_COUNT; i < PAGE_COUNT; i++) {
		union page_dir_entry *dir_entry = &dir->entries[i];
		if (dir_entry->bits.present) {
			struct page_table *table =
				(struct page_table *)(dir_entry->bits.address * PAGE_SIZE);
			for (u32 j = 0; j < PAGE_COUNT; j++) {
				union page_table_entry *table_entry = &table->entries[j];
				if (table_entry->bits.present) {
					u32 paddr = table_entry->bits.address * PAGE_SIZE;
					physical_free(memory_range(paddr, PAGE_SIZE));
				}
			}

			memory_free(&kernel_dir, memory_range((u32)table, sizeof(*table)));
		}
	}

	memory_free(&kernel_dir, memory_range((u32)dir, sizeof(*dir)));
}

struct page_dir *virtual_kernel_dir(void)
{
	return &kernel_dir;
}

/**
 * Memory wrappers
 */

void *memory_alloc(struct page_dir *dir, u32 size, u32 flags)
{
	if (!PAGE_ALIGNED(size) || !size)
		goto err;

	struct memory_range prange = physical_alloc(size);
	if (prange.size == 0)
		goto err;

	u32 vaddr = virtual_alloc(dir, prange, flags).base;
	if (!vaddr) {
		physical_free(prange);
		goto err;
	}

	if (flags & MEMORY_CLEAR)
		memset((void *)vaddr, 0, size);

	return (void *)vaddr;

err:
	print("Memory allocation error!\n");
	return NULL;
}

void *memory_alloc_identity(struct page_dir *dir, u32 flags)
{
	for (u32 i = 1; i < PAGE_KERNEL_COUNT * PAGE_COUNT; i++) {
		struct memory_range range = memory_range(i * PAGE_SIZE, PAGE_SIZE);

		if (!virtual_present(dir, range.base) && !physical_is_used(range)) {
			physical_set_used(range);
			virtual_map(dir, range, range.base, flags);
			if (flags & MEMORY_CLEAR)
				memset((void *)range.base, 0, PAGE_SIZE);
			return (void *)range.base;
		}
	}

	return 0;
}

#define SHARED_MEMORY_MAX 128
struct shared_memory {
	u32 id;
	u32 refs;
	u8 used;
	struct memory_range prange;
};
static struct shared_memory shmem[SHARED_MEMORY_MAX] = { 0 };
res memory_shalloc(struct page_dir *dir, u32 size, u32 *id, u32 flags)
{
	if (!id || !memory_valid(id))
		return -EFAULT;

	*id = 0;

	u32 slot = SHARED_MEMORY_MAX + 1;

	for (u32 i = 0; i < SHARED_MEMORY_MAX; i++) {
		if (!shmem[i].used) {
			slot = i;
			break;
		}
	}

	if (slot >= SHARED_MEMORY_MAX)
		return -ENOMEM;

	void *addr = memory_alloc(dir, size, flags);
	if (!addr)
		return -ENOMEM;

	// TODO: Verify that shid isn't used already
	// TODO: Check for colliding prange
	u32 shid = rdseed() + 1;
	shmem[slot].id = shid;
	shmem[slot].used = 1;
	shmem[slot].prange = memory_range(virtual_to_physical(dir, (u32)addr), size);

	*id = shid;

	return EOK;
}

static struct shared_memory memory_shresolve_range(struct memory_range prange)
{
	for (u32 i = 0; i < SHARED_MEMORY_MAX; i++) {
		if (!shmem[i].used)
			continue;

		struct memory_range shrange = shmem[i].prange;
		if (prange.base < shrange.base + shrange.size &&
		    prange.base + prange.size > shrange.base)
			return shmem[i];
	}

	return (struct shared_memory){ 0 };
}

static struct shared_memory memory_shresolve_id(u32 shid)
{
	for (u32 i = 0; i < SHARED_MEMORY_MAX; i++) {
		if (!shmem[i].used)
			continue;

		if (shmem[i].id == shid)
			return shmem[i];
	}

	return (struct shared_memory){ 0 };
}

res memory_shaccess(struct page_dir *dir, u32 shid, u32 *addr, u32 *size)
{
	if (!addr || !memory_valid(addr) || !size || !memory_valid(size))
		return -EFAULT;

	*addr = 0;
	*size = 0;

	struct shared_memory sh = memory_shresolve_id(shid);
	struct memory_range prange = sh.prange;
	if (sh.used == 0 || prange.base == 0 || prange.size == 0)
		return -ENOENT;

	sh.refs++;

	struct memory_range shrange = virtual_alloc(dir, prange, MEMORY_CLEAR | MEMORY_USER);
	*addr = shrange.base;
	*size = shrange.size;

	return EOK;
}

// TODO: Free by address instead of vrange (combine with shmem map?)
void memory_free(struct page_dir *dir, struct memory_range vrange)
{
	assert(PAGE_ALIGNED(vrange.base) && PAGE_ALIGNED(vrange.size));

	struct memory_range prange =
		memory_range(virtual_to_physical(dir, vrange.base), vrange.size);
	struct shared_memory sh = memory_shresolve_range(prange);
	if (sh.used != 0 && sh.refs > 1) {
		panic("Freeing busy memory!\n");
		return;
	}
	/* sh.used = 0; */

	for (u32 i = 0; i < vrange.size / PAGE_SIZE; i++) {
		u32 vaddr = vrange.base + i * PAGE_SIZE;
		if (virtual_present(dir, vaddr)) {
			struct memory_range page_prange =
				memory_range(virtual_to_physical(dir, vaddr), PAGE_SIZE);
			struct memory_range page_vrange = memory_range(vaddr, PAGE_SIZE);
			physical_free(page_prange);
			virtual_free(dir, page_vrange);
		}
	}
}

void memory_map_identity(struct page_dir *dir, struct memory_range prange, u32 flags)
{
	assert(PAGE_ALIGNED(prange.base) && PAGE_ALIGNED(prange.size));

	physical_set_used(prange);
	virtual_map(dir, prange, prange.base, flags);
	if (flags & MEMORY_CLEAR)
		memset((void *)prange.base, 0, prange.size);
}

void memory_switch_dir(struct page_dir *dir)
{
	paging_switch_dir(virtual_to_physical(&kernel_dir, (u32)dir));
}

void memory_backup_dir(struct page_dir **backup)
{
	struct proc *proc = proc_current();
	struct page_dir *dir = proc ? proc->page_dir : virtual_kernel_dir();
	*backup = dir;
}

static u8 memory_bypass_validity = 0;
void memory_bypass_enable(void)
{
	memory_bypass_validity = 1;
}

void memory_bypass_disable(void)
{
	memory_bypass_validity = 0;
}

u8 memory_is_user(u32 addr)
{
	return PDI(addr) >= PAGE_KERNEL_COUNT;
}

// TODO: Limit by proc stack and data range
u8 memory_valid(const void *addr)
{
	if (proc_current() && !memory_bypass_validity)
		return memory_is_user((u32)addr);
	else
		return 1;
}

struct memory_range memory_range_from(u32 base, u32 size)
{
	u32 align = PAGE_SIZE - base % PAGE_SIZE;

	if (base % PAGE_SIZE == 0) {
		align = 0;
	}

	base += align;
	size -= align;

	size -= size % PAGE_SIZE;

	return memory_range(base, size);
}

struct memory_range memory_range_around(u32 base, u32 size)
{
	u32 align = base % PAGE_SIZE;

	base -= align;
	size += align;

	size += PAGE_SIZE - size % PAGE_SIZE;

	return memory_range(base, size);
}

extern u32 kernel_start;
extern u32 kernel_end;
static struct memory_range kernel_memory_range(void)
{
	return memory_range_around((u32)&kernel_start, (u32)&kernel_end - (u32)&kernel_start);
}

void memory_install(struct mem_info *mem_info, struct vid_info *vid_info)
{
	for (struct mmap_boot *p = mem_info->start; (u32)(p - mem_info->start) < mem_info->size;
	     p++) {
		if (p->hbase || !p->acpi || !p->type)
			continue;

		u32 size = p->lsize;
		if (p->hsize)
			size = U32_MAX - p->lbase;

		/* printf("Memory region: %x-%x\n", p->lbase, p->lbase + size); */
		if (p->type == MEMORY_AVAILABLE) {
			physical_set_free(memory_range_around(p->lbase, size / PAGE_SIZE));
			memory_total += size;
		} else if (p->type == MEMORY_DEFECT) {
			printf("Defect memory at 0x%x-0x%x!\n", p->lbase, p->lbase + size);
		}
	}

	for (u32 i = 0; i < PAGE_KERNEL_COUNT; i++) {
		union page_dir_entry *dir_entry = &kernel_dir.entries[i];
		dir_entry->bits.present = 1;
		dir_entry->bits.writable = 1;
		dir_entry->bits.user = 0;
		dir_entry->bits.address = (u32)&kernel_tables[i] / PAGE_SIZE;
	}

	memory_used = 0;
	printf("Detected memory: %dKiB (%dMiB)\n", memory_total >> 10, memory_total >> 20);

	// Map kernel
	memory_map_identity(&kernel_dir, kernel_memory_range(), MEMORY_NONE);

	// Map kernel stack
	memory_map_identity(&kernel_dir, memory_range_around(STACK_START - STACK_SIZE, STACK_SIZE),
			    MEMORY_NONE);

	// Map framebuffer
	memory_map_identity(&kernel_dir, memory_range_around((u32)vid_info->vbe, 0x1000),
			    MEMORY_NONE);
	fb_map_buffer(virtual_kernel_dir(), vid_info);

	// Unmap NULL byte/page
	struct memory_range zero = memory_range(0, PAGE_SIZE);
	virtual_free(&kernel_dir, zero);
	physical_set_used(zero);

	memory_switch_dir(&kernel_dir);
	paging_enable();
}

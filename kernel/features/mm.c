// Hugely inspired by the implementation in skiftOS: MIT License, Copyright (c) 2020 N. Van Bossuyt
// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <cpu.h>
#include <def.h>
#include <errno.h>
#include <fb.h>
#include <mem.h>
#include <mm.h>
#include <multiboot.h>
#include <print.h>
#include <rand.h>

PROTECTED static struct page_dir kernel_dir ALIGNED(PAGE_SIZE) = { 0 };
static struct page_table kernel_tables[PAGE_KERNEL_COUNT] ALIGNED(PAGE_SIZE) = { 0 };

extern u32 kernel_rw_start;
extern u32 kernel_rw_end;

extern u32 kernel_ro_start;
extern u32 kernel_ro_end;

extern u32 kernel_temp_clear_start;
extern u32 kernel_temp_clear_end;

extern u32 kernel_temp_protect_start;
extern u32 kernel_temp_protect_end;

/**
 * Lowlevel paging
 */

static void paging_switch_dir(u32 dir)
{
	cr3_set(dir);
}

CLEAR extern void paging_invalidate_tlb(void);

CLEAR void paging_disable(void)
{
	cr0_set(cr0_get() | 0x7fffffff);
}

CLEAR void paging_enable(void)
{
	cr0_set(cr0_get() | 0x80010000);
}

static const char *page_fault_section(u32 addr)
{
	const char *section = NULL;
	if (addr == 0)
		section = "NULL";
	else if (addr >= (u32)&kernel_temp_clear_start && addr <= (u32)&kernel_temp_clear_end)
		section = "kernel_temp_clear";
	else if (addr >= (u32)&kernel_temp_protect_start && addr <= (u32)&kernel_temp_protect_end)
		section = "kernel_temp_protect";
	else if (addr >= (u32)&kernel_rw_start && addr <= (u32)&kernel_rw_end)
		section = "kernel_rw";
	else if (addr >= (u32)&kernel_ro_start && addr <= (u32)&kernel_ro_end)
		section = "kernel_ro";
	else
		section = "UNKNOWN";
	return section;
}

void page_fault_handler(struct regs *r)
{
	print("--- PAGE FAULT! ---\n");

	// Check error code
	const char *type = (r->err_code & 1) ? "present" : "non-present";
	const char *operation = (r->err_code & 2) ? "write" : "read";
	const char *super = (r->err_code & 4) ? "User" : "Super";

	// Check cr2 address (virtual and physical)
	u32 vaddr;
	__asm__ volatile("movl %%cr2, %%eax" : "=a"(vaddr));
	struct proc *proc = proc_current();
	struct page_dir *dir = proc && proc->page_dir ? proc->page_dir : &kernel_dir;
	u32 paddr = virtual_to_physical(dir, vaddr);

	// Print!

	printf("%s process tried to %s a %s page at [vaddr=%x; paddr=%x]\n", super, operation, type,
	       vaddr, paddr);

	if (proc && vaddr > proc->regs.ebp - PROC_STACK_SIZE - PAGE_SIZE &&
	    vaddr < proc->regs.ebp + PAGE_SIZE)
		print("Probably a stack overflow\n");

	printf("Sections: [vaddr_section=%s; paddr_section=%s; eip_section=%s]\n",
	       page_fault_section(vaddr), page_fault_section(paddr), page_fault_section(r->eip));

	/* printf("%b\n", virtual_entry(dir, vaddr)->uint); */

	isr_panic(r);
}

/**
 * Physical
 */

static u32 memory_used = 0;
PROTECTED static u32 memory_total = 0;
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

CLEAR void physical_set_total(u32 total)
{
	assert(total > 0);
	memory_total = total;
}

void physical_set_used(struct memory_range range)
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

void physical_set_free(struct memory_range range)
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

union page_table_entry *virtual_entry(struct page_dir *dir, u32 vaddr)
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

	return table_entry;
}

u8 virtual_present(struct page_dir *dir, u32 vaddr)
{
	union page_table_entry *table_entry = virtual_entry(dir, vaddr);
	return !!table_entry;
}

u8 virtual_user_readable(struct page_dir *dir, u32 vaddr)
{
	union page_table_entry *table_entry = virtual_entry(dir, vaddr);
	return table_entry && table_entry->bits.user;
}

u8 virtual_user_writable(struct page_dir *dir, u32 vaddr)
{
	union page_table_entry *table_entry = virtual_entry(dir, vaddr);
	return table_entry && table_entry->bits.user && table_entry->bits.writable;
}

u32 virtual_to_physical(struct page_dir *dir, u32 vaddr)
{
	union page_table_entry *table_entry = virtual_entry(dir, vaddr);
	if (!table_entry)
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
		table_entry->bits.writable = !(flags & MEMORY_READONLY);
		table_entry->bits.user = flags & MEMORY_USER;
		table_entry->bits.address = (prange.base + offset) >> 12;
	}

	paging_invalidate_tlb();
}

void virtual_remap_readonly(struct page_dir *dir, struct memory_range vrange)
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
			table_entry->bits.writable = 0;
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
		memset_user((void *)vaddr, 0, size);

	return (void *)vaddr;

err:
	print("Memory allocation error!\n");
	return NULL;
}

void *memory_alloc_with_boundary(struct page_dir *dir, u32 size, u32 flags)
{
	u32 mem = PAGE_SIZE + (u32)memory_alloc(dir, size + 2 * PAGE_SIZE, flags);
	virtual_remap_readonly(dir, memory_range(mem - PAGE_SIZE, PAGE_SIZE));
	virtual_remap_readonly(dir, memory_range(mem + size, PAGE_SIZE));
	return (void *)mem;
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

	print("Memory allocation error!\n");
	return NULL;
}

void memory_free(struct page_dir *dir, struct memory_range vrange)
{
	assert(PAGE_ALIGNED(vrange.base) && PAGE_ALIGNED(vrange.size));

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

static struct list *memory_objects = NULL;
res memory_sys_alloc(struct page_dir *dir, u32 size, u32 *addr, u32 *id, u8 shared)
{
	if (!memory_writable(addr) || !memory_writable(id))
		return -EFAULT;

	size = PAGE_ALIGN_UP(size);

	u32 vaddr = (u32)memory_alloc(dir, size, MEMORY_CLEAR | MEMORY_USER);
	if (!vaddr)
		return -ENOMEM;

	struct memory_object *obj = zalloc(sizeof(*obj));
	obj->id = rand() + 1;
	obj->prange = memory_range(virtual_to_physical(dir, vaddr), size);
	obj->refs = 1;
	obj->shared = shared;
	list_add(memory_objects, obj);

	struct memory_proc_link *link = zalloc(sizeof(*link));
	link->obj = obj;
	link->vrange = memory_range(vaddr, size);
	list_add(proc_current()->memory, link);

	stac();
	*addr = vaddr;
	*id = obj->id;
	clac();

	return EOK;
}

res memory_sys_free(struct page_dir *dir, u32 addr)
{
	if (!memory_readable((void *)addr))
		return -EFAULT;

	struct list *links = proc_current()->memory;
	struct node *iterator = links->head;
	while (iterator) {
		struct memory_proc_link *link = iterator->data;
		if (link->vrange.base == addr) {
			virtual_free(dir, link->vrange);
			link->obj->refs--;
			if (link->obj->refs == 0) {
				list_remove(memory_objects,
					    list_first_data(memory_objects, link->obj));
				physical_free(link->obj->prange);
				free(link->obj);
			}
			list_remove(links, list_first_data(links, link));
			free(link);
			return EOK;
		}
		iterator = iterator->next;
	}

	return -ENOENT;
}

res memory_sys_shaccess(struct page_dir *dir, u32 id, u32 *addr, u32 *size)
{
	if (!memory_writable(addr) || !memory_writable(size))
		return -EFAULT;

	struct node *iterator = memory_objects->head;
	while (iterator) {
		struct memory_object *obj = iterator->data;
		if (obj->id == id) {
			if (!obj->shared)
				return -EACCES;

			obj->refs++;
			struct memory_range shrange =
				virtual_alloc(dir, obj->prange, MEMORY_CLEAR | MEMORY_USER);

			stac();
			*addr = shrange.base;
			*size = shrange.size;
			clac();

			struct memory_proc_link *link = zalloc(sizeof(*link));
			link->obj = obj;
			link->vrange = shrange;
			list_add(proc_current()->memory, link);

			return EOK;
		}
		iterator = iterator->next;
	}

	stac();
	*addr = 0;
	*size = 0;
	clac();

	return -ENOENT;
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

u8 memory_is_user(const void *addr)
{
	if (!addr)
		return 0;

	return PDI((u32)addr) >= PAGE_KERNEL_COUNT;
}

u8 memory_readable_range(struct memory_range vrange)
{
	if (!vrange.base)
		return 0;

	struct proc *proc = proc_current();
	if (memory_bypass_validity || !proc)
		return 1;

	u32 offset = 0;
	do {
		u8 valid = memory_is_user((void *)(vrange.base + offset)) &&
			   virtual_user_readable(proc->page_dir, vrange.base + offset);
		if (!valid)
			return 0;

		offset += PAGE_SIZE;
	} while (offset < vrange.size);

	return 1;
}

u8 memory_writable_range(struct memory_range vrange)
{
	if (!vrange.base)
		return 0;

	struct proc *proc = proc_current();
	if (memory_bypass_validity || !proc)
		return 1;

	u32 offset = 0;
	do {
		u8 valid = memory_is_user((void *)(vrange.base + offset)) &&
			   virtual_user_writable(proc->page_dir, vrange.base + offset);
		if (!valid)
			return 0;

		offset += PAGE_SIZE;
	} while (offset < vrange.size);

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

CLEAR static struct memory_range kernel_rw_memory_range(void)
{
	return memory_range_around((u32)&kernel_rw_start,
				   (u32)&kernel_rw_end - (u32)&kernel_rw_start);
}

CLEAR static struct memory_range kernel_ro_memory_range(void)
{
	return memory_range_around((u32)&kernel_ro_start,
				   (u32)&kernel_ro_end - (u32)&kernel_ro_start);
}

static void memory_temp_clear(void)
{
	u8 *data = (u8 *)&kernel_temp_clear_start;
	u32 size = (u32)&kernel_temp_clear_end - (u32)&kernel_temp_clear_start;
	memset(data, 0, size);
	memory_free(&kernel_dir, memory_range_around((u32)data, size));
	printf("Cleared %dKiB\n", size >> 10);
}

CLEAR static void memory_temp_protect(void)
{
	u32 data = (u32)&kernel_temp_protect_start;
	u32 size = (u32)&kernel_temp_protect_end - (u32)&kernel_temp_protect_start;
	memory_map_identity(&kernel_dir, memory_range_around((u32)data, size), MEMORY_READONLY);
	printf("Protected %dKiB\n", size >> 10);
}

void memory_user_hook(void)
{
	PROTECTED static u8 called = 0;
	if (!called) {
		called = 1;
		memory_temp_protect();
		memory_temp_clear();
	}
}

CLEAR void memory_install(void)
{
	multiboot_mmap();

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
	memory_map_identity(&kernel_dir, kernel_ro_memory_range(), MEMORY_READONLY);
	memory_map_identity(&kernel_dir, kernel_rw_memory_range(), MEMORY_NONE);

	// Map framebuffer
	memory_map_identity(&kernel_dir, memory_range_around(multiboot_vbe(), 0x1000), MEMORY_NONE);

	// Unmap NULL byte/page
	struct memory_range zero = memory_range(0, PAGE_SIZE);
	virtual_free(&kernel_dir, zero);
	physical_set_used(zero);

	memory_switch_dir(&kernel_dir);
	paging_enable();

	memory_objects = list_new();
}

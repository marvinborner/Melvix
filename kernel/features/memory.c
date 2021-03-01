// Hugely inspired by the implementation in skiftOS: MIT License, Copyright (c) 2020 N. Van Bossuyt
// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <cpu.h>
#include <def.h>
#include <mem.h>
#include <memory.h>

#include <print.h>

/**
 * Paging
 */

void paging_disable(void)
{
	cr0_set(cr0_get() | 0x7fffffff);
}

void paging_enable(void)
{
	cr0_set(cr0_get() | 0x80000000);
}

void paging_switch_dir(u32 dir)
{
	cr3_set(dir);
}

extern void paging_invalidate_tlb(void);

/**
 * Physical
 */

static u32 memory_used = 0;
static u32 memory_total = 0;
static u8 memory[PAGE_COUNT * PAGE_COUNT / 8] = { 0 };
#define PHYSICAL_IS_USED(addr)                                                                     \
	(memory[(u32)(addr) / PAGE_SIZE / 8] & (1 << ((u32)(addr) / PAGE_SIZE % 8)))

#define PHYSICAL_SET_USED(addr)                                                                    \
	(memory[(u32)(addr) / PAGE_SIZE / 8] |= (1 << ((u32)(addr) / PAGE_SIZE % 8)))

#define PHYSICAL_SET_FREE(addr)                                                                    \
	(memory[(u32)(addr) / PAGE_SIZE / 8] &= ~(1 << ((u32)(addr) / PAGE_SIZE % 8)))

u8 physical_is_used(u32 addr, u32 n)
{
	for (u32 i = 0; i < n; i++) {
		if (PHYSICAL_IS_USED(addr + (i * PAGE_SIZE)))
			return 1;
	}
	return 0;
}

void physical_set_used(u32 addr, u32 n)
{
	for (u32 i = 0; i < n; i++) {
		if (!PHYSICAL_IS_USED(addr + (i * PAGE_SIZE))) {
			memory_used += PAGE_SIZE;
			PHYSICAL_SET_USED(addr + (i * PAGE_SIZE));
		}
	}
}

void physical_set_free(u32 addr, u32 n)
{
	for (u32 i = 0; i < n; i++) {
		if (PHYSICAL_IS_USED(addr + (i * PAGE_SIZE))) {
			memory_used -= PAGE_SIZE;
			PHYSICAL_SET_FREE(addr + (i * PAGE_SIZE));
		}
	}
}

u32 physical_alloc(u32 n)
{
	for (u32 i = 0; i < (memory_total / PAGE_SIZE); i++) {
		u32 addr = i * PAGE_SIZE;
		if (!physical_is_used(addr, n)) {
			physical_set_used(addr, n);
			return addr;
		}
	}

	panic("Out of physical memory!\n");
	return 0;
}

void physical_free(u32 addr, u32 n)
{
	physical_set_free(addr, n);
}

/**
 * Virtual
 */

#define PDI(vaddr) ((vaddr) >> 22)
#define PTI(vaddr) (((vaddr) >> 12) & 0x03ff)

static struct page_dir kernel_dir ALIGNED(PAGE_SIZE) = { 0 };
static struct page_table kernel_tables[256] ALIGNED(PAGE_SIZE) = { 0 };

u8 virtual_present(struct page_dir *dir, u32 vaddr)
{
	u32 pdi = PDI(vaddr);
	u32 pti = PTI(vaddr);

	union page_dir_entry *dir_entry = &dir->entries[pdi];
	if (!dir_entry->bits.present)
		return 0;

	struct page_table *table = (struct page_table *)(dir_entry->bits.address * PAGE_SIZE);
	union page_table_entry *table_entry = &table->entries[pti];
	if (!table_entry->bits.present)
		return 0;

	return 1;
}

u32 virtual_to_physical(struct page_dir *dir, u32 vaddr)
{
	u32 pdi = PDI(vaddr);
	u32 pti = PTI(vaddr);

	union page_dir_entry *dir_entry = &dir->entries[pdi];
	struct page_table *table = (struct page_table *)(dir_entry->bits.address * PAGE_SIZE);
	union page_table_entry *table_entry = &table->entries[pti];

	return (table_entry->bits.address * PAGE_SIZE) + (vaddr & (PAGE_SIZE - 1));
}

void memory_alloc_identity(struct page_dir *dir, u32 flags, u32 *out);
void virtual_map(struct page_dir *dir, u32 vaddr, u32 paddr, u32 n, u8 user)
{
	for (u32 i = 0; i < n; i++) {
		u32 offset = i * PAGE_SIZE;
		u32 pdi = PDI(vaddr + offset);
		u32 pti = PTI(vaddr + offset);

		union page_dir_entry *dir_entry = &dir->entries[pdi];
		struct page_table *table =
			(struct page_table *)(dir_entry->bits.address * PAGE_SIZE);
		union page_table_entry *table_entry = &table->entries[pti];

		if (!dir_entry->bits.present) {
			memory_alloc_identity(dir, MEMORY_CLEAR, (u32 *)&table);
			dir_entry->bits.present = 1;
			dir_entry->bits.writable = 1;
			dir_entry->bits.user = user;
			dir_entry->bits.address = (u32)table >> 12;
		}

		table_entry->bits.present = 1;
		table_entry->bits.writable = 1;
		table_entry->bits.user = user;
		table_entry->bits.address = (paddr + offset) >> 12;
	}

	paging_invalidate_tlb();
}

struct memory_range virtual_alloc(struct page_dir *dir, struct memory_range physical_range,
				  u32 flags)
{
	u8 is_user = flags & MEMORY_USER;
	u32 vaddr = 0;
	u32 size = 0;
	for (u32 i = (is_user ? 256 : 1) * PAGE_COUNT;
	     i < (is_user ? PAGE_COUNT : 256) * PAGE_COUNT; i++) {
		u32 addr = i * PAGE_SIZE;
		if (!virtual_present(dir, addr)) {
			if (size == 0)
				vaddr = addr;
			size += PAGE_SIZE;
			if (size == physical_range.size) {
				virtual_map(dir, vaddr, physical_range.base,
					    physical_range.size / PAGE_SIZE, is_user);
				return memory_range(vaddr, size);
			}
		} else {
			size = 0;
		}
	}

	panic("Out of virtual memory!\n");
	return memory_range(0, 0);
}

void virtual_free(struct page_dir *dir, struct memory_range virtual_range)
{
	for (u32 i = 0; i < virtual_range.size / PAGE_SIZE; i++) {
		u32 offset = i * PAGE_SIZE;

		u32 pdi = PDI(virtual_range.base + offset);
		u32 pti = PTI(virtual_range.base + offset);

		union page_dir_entry *dir_entry = &dir->entries[pdi];
		struct page_table *table =
			(struct page_table *)(dir_entry->bits.address * PAGE_SIZE);
		union page_table_entry *table_entry = &table->entries[pti];

		if (table_entry->bits.present)
			table_entry->uint = 0;
	}

	paging_invalidate_tlb();
}

/**
 * Memory wrapper
 */

extern u32 kernel_start;
extern u32 kernel_end;

struct memory_range memory_range_from_address(u32 base, u32 size)
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

struct memory_range memory_range_around_address(u32 base, u32 size)
{
	u32 align = base % PAGE_SIZE;

	base -= align;
	size += align;

	size += PAGE_SIZE - size % PAGE_SIZE;

	return memory_range(base, size);
}

static struct memory_range kernel_memory_range(void)
{
	return memory_range_around_address((u32)&kernel_start,
					   (u32)&kernel_end - (u32)&kernel_start);
}

void memory_map_identity(struct page_dir *dir, struct memory_range range, u32 flags)
{
	assert(PAGE_ALIGNED(range.base) && PAGE_ALIGNED(range.size));

	u32 page_count = range.size / PAGE_SIZE;
	physical_set_used(range.base, page_count);
	virtual_map(dir, range.base, range.base, page_count, flags & MEMORY_USER);

	if (flags & MEMORY_CLEAR)
		memset((void *)range.base, 0, range.size);
}

void memory_map(struct page_dir *dir, struct memory_range range, u32 flags)
{
	assert(PAGE_ALIGNED(range.base) && PAGE_ALIGNED(range.size));

	for (u32 i = 0; i < range.size / PAGE_SIZE; i++) {
		u32 vaddr = range.base + i * PAGE_SIZE;
		if (!virtual_present(dir, vaddr)) {
			u32 paddr = physical_alloc(1);
			virtual_map(dir, vaddr, paddr, 1, flags & MEMORY_USER);
		}
	}

	if (flags & MEMORY_CLEAR)
		memset((void *)range.base, 0, range.size);
}

void memory_alloc_identity(struct page_dir *dir, u32 flags, u32 *out)
{
	for (u32 i = 1; i < 256 * PAGE_COUNT; i++) {
		u32 addr = i * PAGE_SIZE;
		if (!virtual_present(dir, addr) && !physical_is_used(addr, 1)) {
			physical_set_used(addr, 1);
			virtual_map(dir, addr, addr, 1, flags & MEMORY_USER);

			if (flags & MEMORY_CLEAR)
				memset((void *)addr, 0, PAGE_SIZE);

			*out = addr;

			return;
		}
	}

	*out = 0;
	panic("Out of memory!\n");
}

void memory_alloc(struct page_dir *dir, u32 size, u32 flags, u32 *out)
{
	assert(size && PAGE_ALIGNED(size));
	*out = 0;

	u32 page_count = size / PAGE_SIZE;
	u32 paddr = physical_alloc(page_count);
	assert(paddr);
	u32 vaddr = virtual_alloc(dir, memory_range(paddr, size), flags).base;
	assert(vaddr);
	if (flags & MEMORY_CLEAR)
		memset((void *)vaddr, 0, page_count * PAGE_SIZE);
	*out = vaddr;
}

void memory_free(struct page_dir *dir, struct memory_range range)
{
	assert(PAGE_ALIGNED(range.base) && PAGE_ALIGNED(range.size));

	for (u32 i = 0; i < range.size / PAGE_SIZE; i++) {
		u32 vaddr = range.base + i * PAGE_SIZE;
		if (virtual_present(dir, vaddr)) {
			physical_free(virtual_to_physical(dir, vaddr), 1);
			virtual_free(dir, memory_range(vaddr, PAGE_SIZE));
		}
	}
}

struct page_dir *memory_dir_create(void)
{
	struct page_dir *dir = NULL;
	memory_alloc(&kernel_dir, sizeof(*dir), MEMORY_CLEAR, (u32 *)&dir);
	memset(dir, 0, sizeof(*dir));

	for (u32 i = 0; i < 256; i++) {
		union page_dir_entry *entry = &dir->entries[i];
		entry->bits.present = 1;
		entry->bits.writable = 1;
		entry->bits.user = 0;
		entry->bits.address = (u32)&kernel_tables[i] / PAGE_SIZE;
	}

	return dir;
}

void memory_dir_destroy(struct page_dir *dir)
{
	for (u32 i = 256; i < PAGE_COUNT; i++) {
		union page_dir_entry *dir_entry = &dir->entries[i];
		if (dir_entry->bits.present) {
			struct page_table *table =
				(struct page_table *)(dir_entry->bits.address * PAGE_SIZE);
			for (u32 j = 0; j < PAGE_COUNT; j++) {
				union page_table_entry *table_entry = &table->entries[j];
				if (table_entry->bits.present)
					physical_free(table_entry->bits.address * PAGE_SIZE, 1);
			}

			memory_free(&kernel_dir, memory_range((u32)table, sizeof(*table)));
		}
	}
	memory_free(&kernel_dir, memory_range((u32)dir, sizeof(*dir)));
}

void memory_dir_switch(struct page_dir *dir)
{
	paging_switch_dir(virtual_to_physical(&kernel_dir, (u32)dir));
}

void memory_initialize(struct mem_info *mem_info)
{
	for (u32 i = 0; i < 256; i++) {
		union page_dir_entry *entry = &kernel_dir.entries[i];
		entry->bits.present = 1;
		entry->bits.writable = 1;
		entry->bits.user = 0;
		entry->bits.address = (u32)&kernel_tables[i] / PAGE_SIZE;
	}

	// Detect memory using E820 memory map
	for (struct mmap_boot *p = mem_info->start; (u32)(p - mem_info->start) < mem_info->size;
	     p++) {
		if (p->hbase || !p->acpi || !p->type)
			continue;

		u32 size = p->lsize;
		if (p->hsize)
			size = U32_MAX - p->lbase;

		/* printf("Memory region: %x-%x\n", p->lbase, p->lbase + size); */
		if (p->type == MEMORY_AVAILABLE) {
			physical_set_free(p->lbase, size / PAGE_SIZE);
			memory_total += size;
		} else if (p->type == MEMORY_DEFECT) {
			printf("Defect memory at 0x%x-0x%x!\n", p->lbase, p->lbase + size);
		}
	}

	memory_used = 0;
	printf("Detected memory: %dKiB (%dMiB)\n", memory_total >> 10, memory_total >> 20);

	// Map kernel
	memory_map_identity(&kernel_dir, kernel_memory_range(), MEMORY_NONE);

	// Map kernel stack
	memory_map_identity(&kernel_dir,
			    memory_range_around_address(STACK_START - STACK_SIZE, STACK_SIZE),
			    MEMORY_NONE);

	// Map kernel heap
	memory_map_identity(&kernel_dir, memory_range_around_address(HEAP_START, HEAP_INIT_SIZE),
			    MEMORY_NONE);

	// Unmap NULL byte/page
	virtual_free(&kernel_dir, memory_range(0, PAGE_SIZE));
	physical_set_used(0, 1);

	memory_dir_switch(&kernel_dir);
	paging_enable();
}

void paging_install(struct mem_info *mem_info)
{
	memory_initialize(mem_info);
	heap_init(HEAP_START);
}

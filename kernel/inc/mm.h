// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef PAGING_H
#define PAGING_H

#include <boot.h>
#include <def.h>

/**
 * Physical
 */

u32 physical_alloc(u32 n);

/**
 * Virtual
 */

#define PAGE_SIZE 0x1000
#define PAGE_COUNT 1024
#define PAGE_ALIGN(x) ((x) + PAGE_SIZE - ((x) % PAGE_SIZE))
#define PAGE_ALIGNED(x) ((x) % PAGE_SIZE == 0)
#define PAGE_ALIGN_UP(x) (((x) % PAGE_SIZE == 0) ? (x) : (x) + PAGE_SIZE - ((x) % PAGE_SIZE))
#define PAGE_ALIGN_DOWN(x) ((x) - ((x) % PAGE_SIZE))

union page_table_entry {
	struct PACKED {
		u32 present : 1;
		u32 writable : 1;
		u32 user : 1;
		u32 write_through : 1;
		u32 cache_disable : 1;
		u32 accessed : 1;
		u32 dirty : 1;
		u32 attribute : 1;
		u32 global : 1;
		u32 available : 3;
		u32 address : 20;
	} bits;
	u32 uint;
} PACKED;

struct page_table {
	union page_table_entry entries[PAGE_COUNT];
} PACKED;

union page_dir_entry {
	struct PACKED {
		u32 present : 1;
		u32 writable : 1;
		u32 user : 1;
		u32 write_through : 1;
		u32 cache_disable : 1;
		u32 accessed : 1;
		u32 reserved : 1;
		u32 page_size : 1;
		u32 global : 1;
		u32 available : 3;
		u32 address : 20;
	} bits;
	u32 uint;
} PACKED;

struct page_dir {
	union page_dir_entry entries[PAGE_COUNT];
} PACKED;

u32 virtual_to_physical(struct page_dir *dir, u32 vaddr);
void virtual_map(struct page_dir *dir, u32 vaddr, u32 paddr, u32 n, u8 user);
struct memory_range virtual_alloc(struct page_dir *dir, struct memory_range physical_range,
				  u32 flags);
void paging_install(struct mem_info *mem_info);

/**
 * Memory wrappers
 */

#define MEMORY_NONE (0 << 0)
#define MEMORY_USER (1 << 0)
#define MEMORY_CLEAR (1 << 1)
#define memory_range(base, size) ((struct memory_range){ (base), (size) })

struct memory_range {
	u32 base;
	u32 size;
};

struct memory_range memory_range_from_address(u32 base, u32 size);
struct memory_range memory_range_around_address(u32 base, u32 size);

struct page_dir *memory_dir_create(void);
void memory_dir_switch(struct page_dir *dir);
// TODO: Remove these wrappers completely?
/* void memory_alloc(struct page_dir *dir, u32 size, u32 flags, u32 *out); */
/* void memory_map(struct page_dir *dir, struct memory_range range, u32 flags); */
struct page_dir *memory_kernel_dir(void);

#endif

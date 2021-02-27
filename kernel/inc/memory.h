// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef PAGING_H
#define PAGING_H

#include <def.h>

/**
 * Physical
 */

/**
 * Virtual
 */

#define PAGE_SIZE 0x1000
#define PAGE_COUNT 1024
#define PAGE_ALIGN(x) ((x) + PAGE_SIZE - ((x) % PAGE_SIZE))
#define PAGE_ALIGNED(x) ((x) % PAGE_SIZE == 0)

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

void paging_install(void);

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

#endif

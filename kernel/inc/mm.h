// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef PAGING_H
#define PAGING_H

#include <def.h>
#include <drivers/int.h>
#include <errno.h>

struct memory_range {
	u32 base;
	u32 size;
};

/**
 * Lowlevel paging
 */

void paging_disable(void);
void paging_enable(void);

/**
 * Physical
 */

struct memory_range physical_alloc(u32 size);
void physical_free(struct memory_range range);
void physical_set_used(struct memory_range range);
void physical_set_free(struct memory_range range);
void physical_set_total(u32 total);
void physical_print(void);

/**
 * Virtual
 */

#define PAGE_SIZE 0x1000u
#define PAGE_COUNT 1024u
#define PAGE_KERNEL_COUNT 256u
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

union page_table_entry *virtual_entry(struct page_dir *dir, u32 vaddr);
u8 virtual_present(struct page_dir *dir, u32 vaddr) NONNULL;
u32 virtual_to_physical(struct page_dir *dir, u32 vaddr) NONNULL;
u8 virtual_user_readable(struct page_dir *dir, u32 vaddr) NONNULL;
u8 virtual_user_writable(struct page_dir *dir, u32 vaddr) NONNULL;
void virtual_map(struct page_dir *dir, struct memory_range prange, u32 vaddr, u32 flags) NONNULL;
void virtual_remap_readonly(struct page_dir *dir, struct memory_range vrange);
struct memory_range virtual_alloc(struct page_dir *dir, struct memory_range physical_range,
				  u32 flags) NONNULL;
void virtual_free(struct page_dir *dir, struct memory_range vrange) NONNULL;
struct page_dir *virtual_create_dir(void);
void virtual_destroy_dir(struct page_dir *dir) NONNULL;
struct page_dir *virtual_kernel_dir(void);

/**
 * Memory wrappers
 */

struct memory_object {
	u32 id;
	u32 refs;
	u8 shared;
	struct memory_range prange;
};

struct memory_proc_link {
	struct memory_object *obj;
	struct memory_range vrange;
};

#define MEMORY_NONE (0 << 0)
#define MEMORY_USER (1 << 0)
#define MEMORY_CLEAR (1 << 1)
#define MEMORY_READONLY (1 << 2)
#define memory_range(base, size) ((struct memory_range){ (u32)(base), (size) })

struct memory_range memory_range_from(u32 base, u32 size);
struct memory_range memory_range_around(u32 base, u32 size);

void *memory_alloc(struct page_dir *dir, u32 size, u32 flags) NONNULL;
void *memory_alloc_with_boundary(struct page_dir *dir, u32 size, u32 flags) NONNULL;
void *memory_alloc_identity(struct page_dir *dir, u32 flags) NONNULL;
void memory_free(struct page_dir *dir, struct memory_range vrange) NONNULL;
void memory_map_identity(struct page_dir *dir, struct memory_range prange, u32 flags) NONNULL;
void memory_switch_dir(struct page_dir *dir) NONNULL;
void memory_backup_dir(struct page_dir **backup) NONNULL;

// Bypass should almost never be used
void memory_bypass_enable(void);
void memory_bypass_disable(void);

// No NONNULL on verification (for syscalls etc)
u8 memory_is_user(const void *addr);
u8 memory_readable_range(struct memory_range vrange);
u8 memory_writable_range(struct memory_range vrange);
#define memory_readable(addr) memory_readable_range(memory_range((addr), 1))
#define memory_writable(addr) memory_writable_range(memory_range((addr), 1))

// User interface - No NONNULL on syscalls
res memory_sys_alloc(struct page_dir *dir, u32 size, u32 *addr, u32 *id, u8 shared);
res memory_sys_free(struct page_dir *dir, u32 addr);
res memory_sys_shaccess(struct page_dir *dir, u32 id, u32 *addr, u32 *size);

void memory_user_hook(void);
void memory_install(void) NONNULL;

#endif

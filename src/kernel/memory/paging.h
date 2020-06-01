#ifndef MELVIX_PAGING_H
#define MELVIX_PAGING_H

#include <stdint.h>

#define PAGE_SIZE 0x1000
#define PAGE_COUNT 1024
#define SHIFT(address) ((address) >> 12)
#define UNSHIFT(address) ((address) << 12)

int paging_enabled;

struct page_table_entry {
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
} __attribute__((packed));

struct page_table {
	struct page_table_entry entries[PAGE_COUNT];
};

struct page_dir_entry {
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
} __attribute__((packed));

struct page_dir {
	struct page_dir_entry entries[PAGE_COUNT];
};

void paging_install();
void paging_enable();
void paging_disable();
void paging_switch_directory(u32 dir);

struct page_table_entry *paging_get_page(u32 address, struct page_dir *page_dir);
void paging_frame_alloc(struct page_table_entry *page);
void paging_frame_free(struct page_table_entry *page);
struct page_dir *paging_make_dir();
void paging_free_dir();

#endif
#ifndef MELVIX_PAGING_H
#define MELVIX_PAGING_H

#include <stdint.h>

struct page {
	uint32_t present : 1;
	uint32_t rw : 1;
	uint32_t user : 1;
	uint32_t accessed : 1;
	uint32_t dirty : 1;
	uint32_t unused : 7;
	uint32_t frame : 20;
};

struct page_table {
	struct page pages[1024];
};

struct page_directory {
	struct page_table *tables[1024];
};

struct page_directory *paging_root_directory;

struct page_table *get_cr3();
uint32_t get_cr0();

void set_cr3(struct page_directory *dir);
void set_cr0(uint32_t new_cr0);

void paging_switch_directory(struct page_directory *dir);

struct page_directory *paging_make_directory();
struct page_table *paging_make_table();

void paging_install();

void paging_map(struct page_directory *cr3, uint32_t virt, uint32_t phys);
void paging_map_user(struct page_directory *cr3, uint32_t virt, uint32_t phys);

void paging_convert_page(struct page_directory *kdir);

struct page_directory *paging_copy_user_directory(struct page_directory *dir);

#define EMPTY_TAB ((struct page_table *)0x00000002)
#define PAGE_S 0x400000

#endif

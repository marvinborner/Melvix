#ifndef MELVIX_PAGING_H
#define MELVIX_PAGING_H

#include <stdint.h>

struct page {
	u32 present : 1;
	u32 rw : 1;
	u32 user : 1;
	u32 accessed : 1;
	u32 dirty : 1;
	u32 unused : 7;
	u32 frame : 20;
};

struct page_table {
	struct page pages[1024];
};

struct page_directory {
	struct page_table *tables[1024];
};

struct page_directory *paging_root_directory;

struct page_table *get_cr3();
u32 get_cr0();

void set_cr3(struct page_directory *dir);
void set_cr0(u32 new_cr0);

void paging_disable();
void paging_enable();
void paging_switch_directory(struct page_directory *dir);

struct page_directory *paging_make_directory();
struct page_table *paging_make_table();

u32 paging_get_phys(u32 virt);
void paging_install();

void paging_map(struct page_directory *cr3, u32 virt, u32 phys);
void paging_map_user(struct page_directory *cr3, u32 virt, u32 phys);

void paging_convert_page(struct page_directory *kdir);

struct page_directory *paging_copy_user_directory(struct page_directory *dir);

#define EMPTY_TAB ((struct page_table *)0x00000002)
#define PAGE_S 0x400000

#endif
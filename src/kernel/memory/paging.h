#ifndef MELVIX_PAGING_H
#define MELVIX_PAGING_H

#include <stdint.h>

#define PAGE_SIZE 0x400000

#define PD_PRESENT 1 << 0
#define PD_RW 1 << 1
#define PD_USER 1 << 2
#define PD_WRITETHR 1 << 3
#define PD_CACHE_D 1 << 4
#define PD_ACCESSED 1 << 5
#define PD_4M_PAGE 1 << 7

#define PT_PRESENT 1 << 0
#define PT_RW 1 << 1
#define PT_USER 1 << 2
#define PT_WRITETHR 1 << 3
#define PT_CACHE_D 1 << 4
#define PT_ACCESSED 1 << 5
#define PT_DIRTY 1 << 6
#define PT_GLOBAL 1 << 8
#define PT_USED 1 << 9

typedef u32 (*page_tables)[1024] __attribute__((aligned(4096)));

struct page_dir {
	page_tables tables;
};

struct page_dir *current_page_directory;
struct page_dir kernel_page_directory[1024] __attribute__((aligned(4096)));
int paging_enabled;

void paging_install(u32 multiboot_address);
void paging_enable();
void paging_disable();

struct page_dir *paging_make_directory(int user);
void paging_switch_directory(struct page_dir *dir);

void paging_map(u32 phy, u32 virt, u16 flags);
u32 paging_get_phys(u32 virt);
u16 paging_get_flags(u32 virt);
u32 paging_get_used_pages();

void paging_set_flags(u32 virt, u32 count, u16 flags);
void paging_set_flag_up(u32 virt, u32 count, u32 flag);
void paging_set_flag_down(u32 virt, u32 count, u32 flag);
void paging_set_present(u32 virt, u32 count);
void paging_set_absent(u32 virt, u32 count);
void paging_set_used(u32 virt, u32 count);
void paging_set_free(u32 virt, u32 count);

u32 paging_find_pages(u32 count);
u32 paging_alloc_pages(u32 count);

#endif
#ifndef MELVIX_PAGING_H
#define MELVIX_PAGING_H

#include <stdint.h>
#include <kernel/interrupts/interrupts.h>

typedef struct page {
    uint32_t present: 1;
    uint32_t rw: 1;
    uint32_t user: 1;
    uint32_t accessed: 1;
    uint32_t dirty: 1;
    uint32_t unused: 7;
    uint32_t frame: 20;
} page_t;

typedef struct page_table {
    page_t pages[1024];
} page_table_t;

typedef struct page_directory {
    page_table_t *tables[1024];
    uint32_t tablesPhysical[1024];
    uint32_t physicalAddr;
} page_directory_t;

int paging_enabled;

void paging_alloc_frame(page_t *page, int is_kernel, int is_writeable);

void paging_free_frame(page_t *page);

void paging_install();

void paging_switch_directory(page_directory_t *dir);

void paging_enable();

void paging_disable();

page_t *paging_get_page(uint32_t address, int make, page_directory_t *dir);

page_directory_t *paging_clone_directory(page_directory_t *src);

#endif

#ifndef MELVIX_PAGING_H
#define MELVIX_PAGING_H

#include <stdint.h>
#include "../interrupts/interrupts.h"

typedef struct page {
    uint32_t present    : 1;   // Page present in memory
    uint32_t rw         : 1;   // Read-only if clear, readwrite if set
    uint32_t user       : 1;   // Supervisor level only if clear
    uint32_t accessed   : 1;   // Has the page been accessed since last refresh?
    uint32_t dirty      : 1;   // Has the page been written to since last refresh?
    uint32_t unused     : 7;   // Amalgamation of unused and reserved bits
    uint32_t frame      : 20;  // Frame address (shifted right 12 bits)
} page_t;

typedef struct page_table {
    page_t pages[1024];
} page_table_t;

typedef struct page_directory {
    page_table_t *tables[1024];
    uint32_t tablesPhysical[1024];
    uint32_t physicalAddr;
} page_directory_t;

/**
 * Initialize the environment and enables paging
 */
void initialise_paging();

/**
 * Load the page directory into the CR3 register
 * @param new The page directory
 */
void switch_page_directory(page_directory_t *new);

/**
 * Get a specific page pointer
 * @param address The page address
 * @param make If 1 create the page first
 * @param dir The page directory
 * @return The page pointer
 */
page_t *get_page(uint32_t address, int make, page_directory_t *dir);

/**
 * Page fault handler
 * @param r The IRQ registers
 */
void page_fault(struct regs *r);

void alloc_frame(page_t *page, int is_kernel, int is_writeable);

void free_frame(page_t *page);

/**
 * Copy/clone a page directory
 * @param src The page directory
 * @return A new page directory pointer
 */
page_directory_t *clone_directory(page_directory_t *src);

#endif

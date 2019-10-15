#ifndef MELVIX_KHEAP_H
#define MELVIX_KHEAP_H

#include "ordered_array.h"

#define KHEAP_START         0xC0000000
#define KHEAP_INITIAL_SIZE  0x100000

#define HEAP_INDEX_SIZE   0x20000
#define HEAP_MAGIC        0x03A93A90
#define HEAP_MIN_SIZE     0x70000

/**
 * Size information of holes/blocks
 */
typedef struct {
    uint32_t magic;
    unsigned char is_hole; // 1 if hole
    uint32_t size;
} header_t;

typedef struct {
    uint32_t magic;
    header_t *header;
} footer_t;

typedef struct {
    ordered_array_t index;
    uint32_t start_address;
    uint32_t end_address;
    uint32_t max_address;
    unsigned char supervisor;
    unsigned char readonly;
} heap_t;

/**
 * Create a new heap
 * @param start
 * @param end
 * @param max
 * @param supervisor
 * @param readonly
 * @return The heap pointer
 */
heap_t *create_heap(uint32_t start, uint32_t end, uint32_t max, unsigned char supervisor, unsigned char readonly);

/**
 * Allocate a region of memory
 * @param size The size of the memory
 * @param page_align Start the block on a page boundary
 * @param heap The Heap pointer
 * @return
 */
void *alloc(uint32_t size, unsigned char page_align, heap_t *heap);

/**
 * Release an allocated block
 * @param p The block
 * @param heap The heap
 */
void free(void *p, heap_t *heap);

/**
 * Release an allocated block using kheap
 * @param p The block
 */
void kfree(void *p);

/**
 * Allocate a chunk of memory
 * @param sz The size of the memory
 * @param align Start the block on a page boundary
 * @param phys Location of the memory if not 0
 * @return The memory address
 */
uint32_t kmalloc_int(uint32_t sz, int align, uint32_t *phys);

/**
 * Allocate a page-aligned chunk of memory
 * @param sz The size of the memory
 * @return The memory address
 */
uint32_t kmalloc_a(uint32_t sz);

/**
 * Allocate a chunk of memory in a physical address
 * @param sz The size of the memory
 * @param phys The physical address
 * @return The memory address
 */
uint32_t kmalloc_p(uint32_t sz, uint32_t *phys);

/**
 * Allocate a page-aligned chunk of memory in a physical address
 * @param sz The size of the memory
 * @param phys The physical address
 * @return The memory address
 */
uint32_t kmalloc_ap(uint32_t sz, uint32_t *phys);

/**
 * Allocate a chunk of memory (non page-aligned and no physical address)
 * @param sz The size of the memory
 * @return The memory address
 */
uint32_t kmalloc(uint32_t sz);

#endif

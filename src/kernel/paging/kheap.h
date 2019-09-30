#ifndef MELVIX_KHEAP_H
#define MELVIX_KHEAP_H

#include <stdbool.h>
#include <stddef.h>

#define KHEAP_MAGIC    0x13374242
#define KHEAP_MAGIC2 0xDEADBEEF
#define KHEAP_END 0xFFFFDEAD
#define MEM_END 0x8000000

extern unsigned int end;

typedef struct {
    unsigned int magic;
    bool free;
    unsigned int size;
    unsigned int magic2;
} heap_header_t;

typedef struct {
    unsigned int magic;
    unsigned int size;
    unsigned int magic2;
} heap_footer_t;

#define HEAP_S (sizeof(heap_header_t))
#define HEAP_TOTAL (sizeof(heap_footer_t) + HEAP_S)
#define HEAP_MINIMUM 1
#define HEAP_FIND_SIZE (HEAP_TOTAL + HEAP_MINIMUM)

void init_kheap();

void *fmalloc(unsigned int size);

void *kmalloc(unsigned int size);

void *kmalloc_a(unsigned int size);

void kfree(void *ptr);

void *umalloc(size_t size);

void ufree(void *address);

void init_heap(heap_header_t *heap, size_t size);

#define KHEAP_SIZE 0xFFFFF
#define UHEAP_SIZE 0xFFFFF

#endif

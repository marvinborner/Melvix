#ifndef MELVIX_ALLOC_H
#define MELVIX_ALLOC_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define KHEAP_MAGIC 0x04206969
#define KHEAP_MAGIC2 0xDEADBEEF
#define KHEAP_END 0xFFFFDEAD
#define MEM_END 0x8000000

struct heap_header {
	uint32_t magic;
	bool free;
	uint32_t size;
	uint32_t magic2;
};

struct heap_footer {
	uint32_t magic;
	uint32_t size;
	uint32_t magic2;
};

void kheap_init();

void *fmalloc(uint32_t size);
void *kcalloc(uint32_t num, uint32_t size);
void *kmalloc(uint32_t size);
void *kmalloc_a(uint32_t size);
void kfree(void *ptr);

void *umalloc(size_t size);
void *ucalloc(uint32_t num, uint32_t size);
void ufree(void *address);

void init_heap(struct heap_header *heap, size_t size);

#define KHEAP_SIZE 0xFFFFF
#define UHEAP_SIZE 0xFFFFF
#define HEAP_S (sizeof(struct heap_header))
#define HEAP_TOTAL (sizeof(struct heap_footer) + HEAP_S)
#define HEAP_MINIMUM 1
#define HEAP_FIND_SIZE (HEAP_TOTAL + HEAP_MINIMUM)

#endif
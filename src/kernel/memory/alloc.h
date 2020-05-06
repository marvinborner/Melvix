#ifndef MELVIX_ALLOC_H
#define MELVIX_ALLOC_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define KHEAP_MAGIC 0x04206969
#define KHEAP_MAGIC2 0xDEADBEEF
#define KHEAP_END 0xFFFFDEAD
#define MEM_END 0x8000000

struct heap_header {
	u32 magic;
	bool free;
	u32 size;
	u32 magic2;
};

struct heap_footer {
	u32 magic;
	u32 size;
	u32 magic2;
};

void kheap_init();

void *fmalloc(u32 size);
void *kcalloc(u32 num, u32 size);
void *kmalloc(u32 size);
void *kmalloc_a(u32 size);
void kfree(void *ptr);

void *umalloc(u32 size);
void *ucalloc(u32 num, u32 size);
void ufree(void *address);

void init_heap(struct heap_header *heap, u32 size);

#define KHEAP_SIZE 0xFFFFF
#define UHEAP_SIZE 0xFFFFF
#define HEAP_S (sizeof(struct heap_header))
#define HEAP_TOTAL (sizeof(struct heap_footer) + HEAP_S)
#define HEAP_MINIMUM 1
#define HEAP_FIND_SIZE (HEAP_TOTAL + HEAP_MINIMUM)

#endif
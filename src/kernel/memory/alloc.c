#include <stddef.h>
#include <stdint.h>
#include <kernel/memory/paging.h>
#include <kernel/memory/alloc.h>
#include <kernel/system.h>
#include <kernel/lib/lib.h>

extern uint32_t end;
uint32_t placement_address;

struct heap_header *kheap = NULL;
struct heap_header *uheap = NULL;

void kheap_init()
{
	end = &end;
	placement_address = end;
	kheap = (struct heap_header *)fmalloc(KHEAP_SIZE);
	init_heap(kheap, KHEAP_SIZE);

	// Make user heap
	uheap = (struct heap_header *)kmalloc_a(UHEAP_SIZE);
	init_heap(uheap, UHEAP_SIZE);
	paging_map_user(paging_root_directory, (uint32_t)&uheap, (uint32_t)&uheap);
}

void *fmalloc(uint32_t size)
{
	assert(placement_address + size < MEM_END);
	uint32_t hold = placement_address;
	memset((void *)hold, 0, size);
	placement_address += size;
	return (void *)hold;
}

void *kmalloc_a(uint32_t size)
{
	assert(((placement_address & 0xFFFFF000) + 0x1000) + size < MEM_END);
	placement_address &= 0xFFFFF000;
	placement_address += 0x1000;

	uint32_t hold = placement_address;
	placement_address += size;

	return (void *)hold;
}

struct heap_header *find_sized_heap(struct heap_header *heap, size_t size)
{
	while ((heap->size < HEAP_FIND_SIZE + size) || (heap->free != true)) {
		assert(heap->magic == KHEAP_MAGIC);
		assert(heap->magic2 == KHEAP_MAGIC2);
		struct heap_footer *foot =
			(struct heap_footer *)((uint32_t)heap + HEAP_S + heap->size);
		assert(foot->magic == KHEAP_MAGIC);
		assert(foot->magic2 == KHEAP_MAGIC2);

		if (foot->size == KHEAP_END)
			panic("Out of heap space");

		if (foot->size != heap->size)
			panic("Heap footer/header mismatch");

		heap = (struct heap_header *)((uint32_t)foot + sizeof(struct heap_footer));
	}

	return heap;
}

void split_heap(struct heap_header *heap, size_t size)
{
	struct heap_footer *foot = (struct heap_footer *)((uint32_t)heap + HEAP_S + size);
	foot->magic = KHEAP_MAGIC;
	foot->magic2 = KHEAP_MAGIC2;
	foot->size = size;

	size_t new_size = heap->size - HEAP_TOTAL - size;
	heap->size = size;

	heap = (struct heap_header *)((uint32_t)foot + sizeof(struct heap_footer));
	heap->size = new_size;
	heap->free = true;
	heap->magic = KHEAP_MAGIC;
	heap->magic2 = KHEAP_MAGIC2;

	foot = (struct heap_footer *)((uint32_t)heap + HEAP_S + heap->size);
	if ((foot->magic != KHEAP_MAGIC) || (foot->magic2 != KHEAP_MAGIC2)) {
		warn("Invalid footer in split");
	}

	if (foot->size != KHEAP_END)
		foot->size = new_size;
}

void free_internal(struct heap_header *heap, void *address)
{
	struct heap_header *head = (struct heap_header *)((uint32_t)address - HEAP_S);
	if (head == heap) {
		//warn("Can't collapse top of heap"); // TODO: Fix "can't collapse top of heap" at start
		head->free = true;
		return;
	}

	if ((head->magic != KHEAP_MAGIC) || (head->magic2 != KHEAP_MAGIC2)) {
		warn("Invalid header in heap");
		return;
	}

	struct heap_footer *foot = (struct heap_footer *)((uint32_t)head + HEAP_S + head->size);
	if ((foot->magic != KHEAP_MAGIC) || (foot->magic2 != KHEAP_MAGIC2))
		panic("Bad heap call");

	foot = (struct heap_footer *)((uint32_t)head - sizeof(struct heap_footer));
	if ((foot->magic != KHEAP_MAGIC) || (foot->magic2 != KHEAP_MAGIC2)) {
		warn("Invalid footer in heap");
		return;
	}

	if (foot->size == KHEAP_END)
		panic("Impossible condition for heap");

	heap = (struct heap_header *)((uint32_t)foot - foot->size - HEAP_S);
	if ((heap->magic != KHEAP_MAGIC) || (heap->magic2 != KHEAP_MAGIC2)) {
		warn("Invalid parent in heap");
		return;
	}

	foot = (struct heap_footer *)((uint32_t)heap + (heap->size + head->size + HEAP_TOTAL) +
				      HEAP_S);
	if ((foot->magic != KHEAP_MAGIC) || (foot->magic2 != KHEAP_MAGIC2)) {
		panic("Fatal arithmetic error in free() call");
		return;
	}

	heap->size += head->size + HEAP_TOTAL;
	foot->size = heap->size;
}

void *malloc_internal(struct heap_header *heap, size_t size)
{
	heap = find_sized_heap(heap, size + 8);
	heap->free = false;
	split_heap(heap, size);
	return (void *)((uint32_t)heap + HEAP_S);
}

void init_heap(struct heap_header *heap, size_t size)
{
	heap->magic = KHEAP_MAGIC;
	heap->magic2 = KHEAP_MAGIC2;
	heap->free = true;
	heap->size = size - HEAP_TOTAL;

	struct heap_footer *foot = (struct heap_footer *)((uint32_t)heap + HEAP_S + heap->size);
	foot->magic = KHEAP_MAGIC;
	foot->magic2 = KHEAP_MAGIC2;
	foot->size = KHEAP_END;
}

void *kmalloc(uint32_t size)
{
	if (kheap == NULL)
		return fmalloc(size);

	return malloc_internal(kheap, size);
}

void *kcalloc(uint32_t num, uint32_t size)
{
	void *ptr = kmalloc(num * size);
	memset(ptr, 0, num * size);
	return ptr;
}

void kfree(void *address)
{
	if (kheap == NULL)
		return;

	free_internal(kheap, address);
}

void *umalloc(size_t size)
{
	return malloc_internal(uheap, size);
}

void *ucalloc(uint32_t num, uint32_t size)
{
	void *ptr = umalloc(num * size);
	memset(ptr, 0, num * size);
	return ptr;
}

void ufree(void *address)
{
	free_internal(uheap, address);
}
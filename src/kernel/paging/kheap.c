#include "kheap.h"
#include "paging.h"
#include "../lib/lib.h"
#include "../system.h"

unsigned int placement_address;

heap_header_t *kheap = NULL;
heap_header_t *uheap = NULL;

void init_kheap() {
    end = (unsigned int) &end;
    placement_address = end;

    kheap = (heap_header_t *) fmalloc(KHEAP_SIZE);
    init_heap(kheap, KHEAP_SIZE);

    uheap = (heap_header_t *) kmalloc_a(UHEAP_SIZE);
    init_heap(uheap, UHEAP_SIZE);
    vpage_map_user(root_vpage_dir, (unsigned int) &uheap, (unsigned int) &uheap);
}

void *fmalloc(unsigned int size) {
    assert(placement_address + size < MEM_END);
    unsigned int hold = placement_address;
    memory_set((void *) hold, 0, size);
    placement_address += size;
    return (void *) hold;
}

void *kmalloc_a(unsigned int size) {
    //assert(((placement_address & 0xFFFFF000) + 0x1000) + size < MEM_END);
    placement_address &= 0xFFFFF000;
    placement_address += 0x1000;

    unsigned int hold = placement_address;
    placement_address += size;

    return (void *) hold;
}

heap_header_t *find_sized_heap(heap_header_t *heap, size_t size) {
    while ((heap->size < HEAP_FIND_SIZE + size) || (heap->free != 1)) {
        assert(heap->magic == KHEAP_MAGIC);
        assert(heap->magic2 == KHEAP_MAGIC2);
        heap_footer_t *foot = (heap_footer_t *) ((unsigned int) heap + HEAP_S + heap->size);
        assert(foot->magic == KHEAP_MAGIC);
        assert(foot->magic2 == KHEAP_MAGIC2);

        if (foot->size == KHEAP_END)
            panic("out of heap space");

        if (foot->size != heap->size)
            panic("heap footer/header mismatch");

        heap = (heap_header_t *) ((unsigned int) foot + sizeof(heap_footer_t));
    }

    return heap;
}

void split_heap(heap_header_t *heap, size_t size) {
    heap_footer_t *foot = (heap_footer_t *) ((unsigned int) heap + HEAP_S + size);
    foot->magic = KHEAP_MAGIC;
    foot->magic2 = KHEAP_MAGIC2;
    foot->size = size;

    size_t new_size = heap->size - HEAP_TOTAL - size;
    heap->size = size;

    heap = (heap_header_t *) ((unsigned int) foot + sizeof(heap_footer_t));
    heap->size = new_size;
    heap->free = 1;
    heap->magic = KHEAP_MAGIC;
    heap->magic2 = KHEAP_MAGIC2;

    foot = (heap_footer_t *) ((unsigned int) heap + HEAP_S + heap->size);
    if ((foot->magic != KHEAP_MAGIC) || (foot->magic2 != KHEAP_MAGIC2)) {
        warn("invalid footer in split");
        // dump_struct(foot, sizeof(heap_footer_t));
    }

    if (foot->size != KHEAP_END)
        foot->size = new_size;
}

void free_internal(heap_header_t *heap, void *address) {
    heap_header_t *head = (heap_header_t *) ((unsigned int) address - HEAP_S);
    if (head == heap) {
        warn("can't collapse top of heap");
        head->free = 1;
        return;
    }

    if ((head->magic != KHEAP_MAGIC) || (head->magic2 != KHEAP_MAGIC2)) {
        //warn("invalid header in heap");
        //dump_struct(head, sizeof(heap_header_t));
        return;
    }

    heap_footer_t *foot = (heap_footer_t *) ((unsigned int) head + HEAP_S + head->size);
    if ((foot->magic != KHEAP_MAGIC) || (foot->magic2 != KHEAP_MAGIC2))
        panic("bad heap in free() call");

    foot = (heap_footer_t *) ((unsigned int) head - sizeof(heap_footer_t));
    if ((foot->magic != KHEAP_MAGIC) || (foot->magic2 != KHEAP_MAGIC2)) {
        //warn("invalid footer in heap");
        return;
    }

    if (foot->size == KHEAP_END)
        panic("impossible condition for heap");

    heap = (heap_header_t *) ((unsigned int) foot - foot->size - HEAP_S);
    if ((heap->magic != KHEAP_MAGIC) || (heap->magic2 != KHEAP_MAGIC2)) {
        warn("invalid parent in heap");
        return;
    }

    foot = (heap_footer_t *) ((unsigned int) heap + (heap->size + head->size + HEAP_TOTAL) + HEAP_S);
    if ((foot->magic != KHEAP_MAGIC) || (foot->magic2 != KHEAP_MAGIC2)) {
        return;
    }

    heap->size += head->size + HEAP_TOTAL;
    foot->size = heap->size;
}

void *malloc_internal(heap_header_t *heap, size_t size) {
    heap = find_sized_heap(heap, size + 8);
    heap->free = 0;
    split_heap(heap, size);
    return (void *) ((unsigned int) heap + HEAP_S);
}

void init_heap(heap_header_t *heap, size_t size) {
    heap->magic = KHEAP_MAGIC;
    heap->magic2 = KHEAP_MAGIC2;
    heap->free = 1;
    heap->size = size - HEAP_TOTAL;

    heap_footer_t *foot = (heap_footer_t *) ((unsigned int) heap + HEAP_S + heap->size);
    foot->magic = KHEAP_MAGIC;
    foot->magic2 = KHEAP_MAGIC2;
    foot->size = KHEAP_END;
}

void *kmalloc(unsigned int size) {
    if (kheap == NULL)
        return fmalloc(size);

    return malloc_internal(kheap, size);
}

void kfree(void *address) {
    if (kheap == NULL)
        return;

    free_internal(kheap, address);
}

void *umalloc(size_t size) {
    return malloc_internal(uheap, size);
}

void ufree(void *address) {
    free_internal(uheap, address);
}
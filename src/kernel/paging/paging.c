#include "paging.h"
#include "kheap.h"
#include "../system.h"
#include "../lib/lib.h"
#include "../io/io.h"

page_directory_t *kernel_directory = 0;
page_directory_t *current_directory = 0;
uint32_t *frames;
uint32_t nframes;

extern uint32_t placement_address;
extern heap_t *kheap;

#define INDEX_FROM_BIT(a) (a/(8*4))
#define OFFSET_FROM_BIT(a) (a%(8*4))

static void set_frame(uint32_t frame_addr) {
    uint32_t frame = frame_addr / 0x1000;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    frames[idx] |= (0x1 << off);
}

static void clear_frame(uint32_t frame_addr) {
    uint32_t frame = frame_addr / 0x1000;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    frames[idx] &= ~(0x1 << off);
}

/*static uint32_t test_frame(uint32_t frame_addr) {
    uint32_t frame = frame_addr / 0x1000;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    return (frames[idx] & (0x1 << off));
}*/

static uint32_t first_frame() {
    uint32_t i, j;
    for (i = 0; i < INDEX_FROM_BIT(nframes); i++) {
        if (frames[i] != 0xFFFFFFFF) {
            for (j = 0; j < 32; j++) {
                uint32_t toTest = 0x1 << j;
                if (!(frames[i] & toTest)) {
                    return i * 4 * 8 + j;
                }
            }
        }
    }
    return -1;
}

void alloc_frame(page_t *page, int is_kernel, int is_writeable) {
    if (page->frame != 0) {
        return;
    } else {
        uint32_t idx = first_frame();
        if (idx == (uint32_t) -1) panic("No free frames!");
        set_frame(idx * 0x1000);
        page->present = 1;
        page->rw = (is_writeable == 1) ? 1 : 0;
        page->user = (is_kernel == 1) ? 0 : 1;
        page->frame = idx;
    }
}

void free_frame(page_t *page) {
    uint32_t frame;
    if (!(frame = page->frame)) {
        return;
    } else {
        clear_frame(frame);
        page->frame = 0x0;
    }
}

void initialise_paging() {
    uint32_t mem_end_page = 0x1000000;
    nframes = mem_end_page / 0x1000;
    frames = (uint32_t *) kmalloc(INDEX_FROM_BIT(nframes));
    memory_set(frames, 0, INDEX_FROM_BIT(nframes));

    kernel_directory = (page_directory_t *) kmalloc_a(sizeof(page_directory_t));
    memory_set(kernel_directory, 0, sizeof(page_directory_t));
    kernel_directory->physicalAddr = (uint32_t) kernel_directory->tablesPhysical;

    unsigned int i = 0;
    for (i = KHEAP_START; i < KHEAP_START + KHEAP_INITIAL_SIZE; i += 0x1000)
        get_page(i, 1, kernel_directory);

    i = 0;
    while (i < 0x400000) {
        alloc_frame(get_page(i, 1, kernel_directory), 0, 0);
        i += 0x1000;
    }

    for (i = KHEAP_START; i < KHEAP_START + KHEAP_INITIAL_SIZE; i += 0x1000)
        alloc_frame(get_page(i, 1, kernel_directory), 0, 0);

    irq_install_handler(14, page_fault);
    switch_page_directory(kernel_directory);
    kheap = create_heap(KHEAP_START, KHEAP_START + KHEAP_INITIAL_SIZE, 0xCFFFF000, 0, 0);
    current_directory = clone_directory(kernel_directory);
    switch_page_directory(current_directory);
}

void switch_page_directory(page_directory_t *dir) {
    current_directory = dir;
    asm volatile("mov %0, %%cr3"::"r"(dir->physicalAddr));
    uint32_t cr0;
    asm volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0"::"r"(cr0));
}

page_t *get_page(uint32_t address, int make, page_directory_t *dir) {
    address /= 0x1000;
    uint32_t table_idx = address / 1024;

    if (dir->tables[table_idx]) {
        return &dir->tables[table_idx]->pages[address % 1024];
    } else if (make) {
        uint32_t tmp;
        dir->tables[table_idx] = (page_table_t *) kmalloc_ap(sizeof(page_table_t), &tmp);
        memory_set(dir->tables[table_idx], 0, 0x1000);
        dir->tablesPhysical[table_idx] = tmp | 0x7;
        return &dir->tables[table_idx]->pages[address % 1024];
    } else {
        return 0;
    }
}

void page_fault(struct regs *r) {
    uint32_t faulting_address;
    asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

    int present = !(r->err_code & 0x1);
    int rw = r->err_code & 0x2;
    int us = r->err_code & 0x4;
    int reserved = r->err_code & 0x8;
    // int id = r->err_code & 0x10;

    serial_write("Page fault! ( ");
    if (present) serial_write("present ");
    if (rw) serial_write("read-only ");
    if (us) serial_write("user-mode ");
    if (reserved) serial_write("reserved ");
    serial_write(") at 0x");
    serial_write_hex(faulting_address);
    serial_write(" - EIP: ");
    serial_write_hex(r->eip);
    serial_write("\n");
    panic("Page fault");
}

static page_table_t *clone_table(page_table_t *src, uint32_t *physAddr) {
    page_table_t *table = (page_table_t *) kmalloc_ap(sizeof(page_table_t), physAddr);
    memory_set(table, 0, sizeof(page_directory_t));

    int i;
    for (i = 0; i < 1024; i++) {
        if (!src->pages[i].frame)
            continue;
        alloc_frame(&table->pages[i], 0, 0);
        if (src->pages[i].present) table->pages[i].present = 1;
        if (src->pages[i].rw) table->pages[i].rw = 1;
        if (src->pages[i].user) table->pages[i].user = 1;
        if (src->pages[i].accessed)table->pages[i].accessed = 1;
        if (src->pages[i].dirty) table->pages[i].dirty = 1;
        extern void copy_page_physical(int a, int b);
        copy_page_physical(src->pages[i].frame * 0x1000, table->pages[i].frame * 0x1000);
    }
    return table;
}

page_directory_t *clone_directory(page_directory_t *src) {
    uint32_t phys;
    page_directory_t *dir = (page_directory_t *) kmalloc_ap(sizeof(page_directory_t), &phys);
    memory_set(dir, 0, sizeof(page_directory_t));
    uint32_t offset = (uint32_t) dir->tablesPhysical - (uint32_t) dir;
    dir->physicalAddr = phys + offset;

    for (int i = 0; i < 1024; i++) {
        if (!src->tables[i])
            continue;

        if (kernel_directory->tables[i] == src->tables[i]) {
            dir->tables[i] = src->tables[i];
            dir->tablesPhysical[i] = src->tablesPhysical[i];
        } else {
            uint32_t phys;
            dir->tables[i] = clone_table(src->tables[i], &phys);
            dir->tablesPhysical[i] = phys | 0x07;
        }
    }
    return dir;
}

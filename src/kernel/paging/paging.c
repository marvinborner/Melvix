#include <stdint.h>

uint64_t page_dir_ptr_tab[4] __attribute__((aligned(0x20)));
uint64_t page_dir[512] __attribute__((aligned(0x1000)));
uint64_t page_tab[512] __attribute__((aligned(0x1000)));

void enable_paging() {
    page_dir_ptr_tab[0] = (uint64_t) &page_dir | 1;
    page_dir[0] = (uint64_t) &page_tab | 3;

    unsigned int i, address = 0;
    for (i = 0; i < 512; i++) {
        page_tab[i] = address | 3;
        address = address + 0x1000;
    }

    asm volatile ("cli");
    asm volatile ("movl %cr4, %eax; bts $5, %eax; movl %eax, %cr4"); // CRASH
    asm volatile ("movl %%eax, %%cr3"::"a" (&page_dir_ptr_tab));
    asm volatile ("movl %cr0, %eax; orl $0x80000000, %eax; movl %eax, %cr0;");
    asm volatile ("sti");
}
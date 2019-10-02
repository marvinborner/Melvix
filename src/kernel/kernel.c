#include "graphics/vesa.h"
#include "graphics/graphics.h"
#include "gdt/gdt.h"
#include "interrupts/interrupts.h"
#include "input/input.h"
#include "timer/timer.h"
#include "paging/paging.h"
#include "paging/kheap.h"

void init() {
    gdt_install();
    idt_install();
    isrs_install();
    irq_install();
    timer_install();
    terminal_initialize();
    // init_kheap();
    // page_init();
    keyboard_install();
    // mouse_install();
    asm volatile ("sti");
}

void kernel_main(void) {
    set_optimal_resolution();
    // vbe_set_mode(0x11B);
    init();
    terminal_write_string("Melvix loaded successfully!\n");
    terminal_write_string("Loading VESA!\n");

    // __asm__  ("div %0" :: "r"(0)); // Exception testing x/0
    for (;;);
}
#include "graphics/vesa.h"
#include "graphics/graphics.h"
#include "gdt/gdt.h"
#include "interrupts/interrupts.h"
#include "input/input.h"
#include "io/io.h"
#include "timer/timer.h"
#include "paging/paging.h"
#include "paging/kheap.h"

void init() {
    timer_install();
    gdt_install();
    idt_install();
    isrs_install();
    irq_install();
    init_serial();
    // terminal_initialize(); // TODO: Replace VGA functions with VESA
    // init_kheap();
    // page_init();
    // keyboard_install();
    // mouse_install();
    asm volatile ("sti");
}

void kernel_main(void) {
    set_optimal_resolution();
    init();
    // info("Melvix loaded successfully!\n\n");
    // info("Loading VESA...");

    vesa_draw_string("test");

    if (vesa_available) {
        write_serial("Loaded VESA!");
    } else {
        write_serial("VESA loading failed!");
    }

    // __asm__  ("div %0" :: "r"(0)); // Exception testing x/0
    loop:
    asm volatile ("hlt");
    goto loop;
}

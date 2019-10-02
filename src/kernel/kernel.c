#include "graphics/vesa.h"
#include "graphics/graphics.h"
#include "gdt/gdt.h"
#include "interrupts/interrupts.h"
#include "input/input.h"
#include "timer/timer.h"
#include "paging/paging.h"
#include "paging/kheap.h"

void init() {
    timer_install();
    gdt_install();
    idt_install();
    isrs_install();
    irq_install();
    // terminal_initialize(); // TODO: Re[ace VGA functions with VESA
    init_kheap();
    page_init();
    keyboard_install();
    // mouse_install();
    asm volatile ("sti");
}

void kernel_main(void) {
    set_optimal_resolution();
    init();
    info("Melvix loaded successfully!");
    info("Loading VESA...");

    if (vesa_available) {
        info("Loaded VESA!");
    } else {
        warn("VESA loading failed!");
    }

    // __asm__  ("div %0" :: "r"(0)); // Exception testing x/0
    for (;;);
}
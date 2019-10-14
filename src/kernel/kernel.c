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
    initialise_paging();
    idt_install();
    isrs_install();
    irq_install();
    init_serial();
    set_optimal_resolution();
    // terminal_initialize(); // TODO: Replace VGA functions with VESA
    asm volatile ("sti");
}

void kernel_main(void) {
    init();

    // vesa_draw_string("This is a testing text!");

    if (vesa_available) {
        serial_write("Loaded VESA!\n");
    } else {
        serial_write("VESA loading failed!\n");
    }

    // __asm__  ("div %0" :: "r"(0)); // Exception testing x/0
    loop:
    asm volatile ("hlt");
    goto loop;
}

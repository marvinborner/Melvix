#include <kernel/graphics/vesa.h>
#include <kernel/gdt/gdt.h>
#include <kernel/interrupts/interrupts.h>
#include <kernel/io/io.h>
#include <kernel/timer/timer.h>
#include <kernel/paging/paging.h>
#include <kernel/input/input.h>

void init() {
    timer_install();
    gdt_install();
    paging_install();
    keyboard_install();
    idt_install();
    isrs_install();
    irq_install();
    init_serial();
    set_optimal_resolution();
    asm volatile ("sti");
}

void kernel_main(void) {
    init();

    vesa_draw_string("This is a testing text!");

    if (vesa_available) {
        serial_write("Loaded VESA!\n");
    } else {
        serial_write("VESA loading failed!\n");
        switch_to_vga();
    }

    // __asm__  ("div %0" :: "r"(0)); // Exception testing x/0
    loop:
    asm volatile ("hlt");
    goto loop;
}

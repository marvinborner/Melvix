#include "graphics/vesa.h"
#include "graphics/graphics.h"
#include "gdt/gdt.h"
#include "interrupts/interrupts.h"
#include "input/input.h"
#include "timer/timer.h"

void init() {
    asm volatile ("sti");
    gdt_install();
    idt_install();
    isrs_install();
    irq_install();
    timer_install();
    terminal_initialize();
    keyboard_install();
    // mouse_install();
}

void kernel_main(void) {
    // vbe_set_mode(0x11B);
    set_optimal_resolution();

    terminal_write_string("Melvix loaded successfully!\n");

    // __asm__  ("div %0" :: "r"(0)); // Exception testing x/0
}
#include "graphics/vesa.h"
#include "graphics/graphics.h"
#include "gdt/gdt.h"
#include "interrupts/interrupts.h"
#include "input/input.h"
#include "timer/timer.h"

void kernel_main(void) {
    asm volatile ("sti");
    gdt_install();
    idt_install();
    isrs_install();
    irq_install();
    timer_install();

    // vbe_set_mode(0x11B); // 1280x1024

    terminal_initialize();
    terminal_write_string("Melvix loaded successfully!\n");

    set_optimal_resolution();
    keyboard_install();
    // mouse_install();

    // __asm__  ("div %0" :: "r"(0)); // Exception testing x/0
}
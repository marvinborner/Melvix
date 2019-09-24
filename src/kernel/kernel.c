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

    terminal_initialize();
    terminal_write_string("Melvix loaded successfully!\n");
    terminal_write_string((const char *) best.height);

    timer_install();
    keyboard_install();
    // mouse_install();
    // __asm__  ("div %0" :: "r"(0)); // Exception testing x/0
}
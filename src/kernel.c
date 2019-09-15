#include "graphics/vga.h"
#include "gdt/gdt.h"
#include "interrupts/interrupts.h"
#include "input/input.h"
#include "timer/timer.h"

void kernel_main(void) {
    gdt_install();
    idt_install();
    isrs_install();
    irq_install();

    __asm__ __volatile__ ("sti");

    terminal_initialize();
    timer_install();
    keyboard_install();
    mouse_install();

    terminal_write_string("Melvix loaded successfully!\n");
    // __asm__  ("div %0" :: "r"(0)); // Exception testing x/0
}
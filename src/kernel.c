#include "graphics/vga.h"
#include "gdt/gdt.h"
#include "interrupts/interrupts.h"

void kernel_main(void) {
    gdt_install();
    idt_install();
    isrs_install();
    terminal_initialize();
    terminal_write_string("Melvix loaded successfully!\nTest");
    // __asm__  ("div %0" :: "r"(0)); Exception testing x/0
}
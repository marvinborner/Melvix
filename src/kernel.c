#include "graphics/vga.h"
#include "gdt/gdt.h"
#include "idt/idt.h"

void kernel_main(void) {
    gdt_install();
    idt_install();
    terminal_initialize();
    terminal_write_string("Melvix loaded successfully!\nTest");
}
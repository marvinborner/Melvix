#include <kernel/graphics/vesa.h>
#include <kernel/gdt/gdt.h>
#include <kernel/interrupts/interrupts.h>
#include <kernel/io/io.h>
#include <kernel/timer/timer.h>
#include <kernel/paging/paging.h>
#include <kernel/input/input.h>

void init() {
    vga_log("Installing basic features of Melvix...", 0);
    timer_install();
    gdt_install();
    init_serial();
    paging_install();
    keyboard_install();
    idt_install();
    isrs_install();
    irq_install();
    set_optimal_resolution();
    asm volatile ("sti");
}

void kernel_main(void) {
    init();

    // __asm__  ("div %0" :: "r"(0)); // Exception testing x/0
    loop:
    asm volatile ("hlt");
    goto loop;
}

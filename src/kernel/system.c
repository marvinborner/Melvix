#ifndef MELVIX_SYSTEM_H
#define MELVIX_SYSTEM_H

#include <kernel/timer/timer.h>
#include <kernel/io/io.h>
#include <kernel/graphics/vesa.h>

void kernel_time() {
    vesa_draw_string("\n");
    vesa_draw_string("[");
    vesa_draw_number(get_time());
    vesa_draw_string("] ");
}

void info(char *msg) {
    // terminal_set_color(9);
    kernel_time();
    vesa_draw_string("INFO: ");
    vesa_draw_string(msg);
    vesa_draw_string("\n");
    // terminal_set_color(7);
}

void warn(char *msg) {
    // terminal_set_color(6);
    kernel_time();
    vesa_draw_string("WARNING: ");
    vesa_draw_string(msg);
    vesa_draw_string("\n");
    // terminal_set_color(7);
}

void panic(char *msg) {
    asm volatile ("cli");
    // terminal_set_color(4);
    kernel_time();
    serial_write("PANIC: ");
    serial_write(msg);
    serial_write(" - System Halted!\n");
    loop:
    asm volatile ("hlt");
    goto loop;
}

void assert(int x) {
    if (x == 0) {
        panic("Assertion failed");
    }
}

#endif

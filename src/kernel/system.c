#include <kernel/timer/timer.h>
#include <kernel/io/io.h>
#include <kernel/graphics/vesa.h>
#include <kernel/lib/lib.h>

char *vga_buffer = (char *) 0x500;

void vga_log(char *msg, int line) {
    uint16_t *terminal_buffer = (uint16_t *) 0xB8000;
    for (size_t i = 0; i < strlen(msg); i++)
        terminal_buffer[line * 80 + i] = (uint16_t) msg[i] | (uint16_t) 0x700;
    char string[80];
    char time[8];
    strcpy(string, "[");
    strcat(string, itoa((int) get_time(), time));
    strcat(string, "] ");
    strcat(string, "INFORMATION: ");
    strcat(string, msg);
    strcat(string, "\n");
    strcat(vga_buffer, string);
}

void kernel_time() {
    vesa_draw_string("\n");
    vesa_draw_string("[");
    vesa_draw_number((int) get_time());
    vesa_draw_string("] ");
}

void log(char *msg) {
    vesa_set_color(vesa_dark_white);
    kernel_time();
    vesa_draw_string(msg);
    vesa_set_color(default_text_color);
}

void info(char *msg) {
    vesa_set_color(vesa_blue);
    kernel_time();
    vesa_draw_string("INFORMATION: ");
    vesa_draw_string(msg);
    vesa_set_color(default_text_color);
}

void warn(char *msg) {
    vesa_set_color(vesa_dark_yellow);
    kernel_time();
    vesa_draw_string("WARNING: ");
    vesa_draw_string(msg);
    vesa_set_color(default_text_color);
}

void panic(char *msg) {
    asm volatile ("cli");
    vesa_set_color(vesa_dark_red);
    kernel_time();
    serial_write("\nPANIC: ");
    serial_write(msg);
    serial_write(" - System halted!\n");
    vesa_draw_string("PANIC: ");
    vesa_draw_string(msg);
    vesa_draw_string(" - System halted!\n");
    loop:
    asm volatile ("hlt");
    goto loop;
}

void assert(int x) {
    if (x == 0) {
        panic("Assertion failed");
    }
}

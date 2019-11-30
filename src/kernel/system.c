#include <kernel/timer/timer.h>
#include <kernel/io/io.h>
#include <kernel/graphics/vesa.h>
#include <mlibc/string.h>
#include <mlibc/stdlib.h>
#include <kernel/paging/paging.h>
#include <kernel/interrupts/interrupts.h>
#include <mlibc/stdio.h>
#include <stdarg.h>

char *vga_buffer = (char *) 0x500;

void vga_clear()
{
    uint16_t *terminal_buffer = (uint16_t *) 0xB8000;
    for (size_t y = 0; y < 25; y++)
        for (size_t x = 0; x < 80; x++)
            terminal_buffer[y * 80 + x] = 0 | (uint16_t) 0x700;
}

void vga_log(char *msg, int line)
{
    if (line == 0) vga_clear();
    uint16_t *terminal_buffer = (uint16_t *) 0xB8000;
    for (size_t i = 0; i < strlen(msg); i++)
        terminal_buffer[line * 80 + i] = (uint16_t) msg[i] | (uint16_t) 0x700;
    char string[80];
    strcpy(string, "[");
    strcat(string, itoa((int) get_time()));
    strcat(string, "] ");
    strcat(string, "INFO: ");
    strcat(string, msg);
    strcat(string, "\n");
    strcat(vga_buffer, string);
}

void kernel_time()
{
    printf("[%d] ", (int) get_time());
}

void debug(const char *fmt, ...)
{
    vesa_set_color(vesa_dark_white);
    kernel_time();
    printf("DEBG: ");

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    vesa_set_color(default_text_color);
    writec('\n');
}

void info(const char *fmt, ...)
{
    vesa_set_color(vesa_blue);
    kernel_time();
    printf("INFO: ");

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    vesa_set_color(default_text_color);
    writec('\n');
}

void warn(const char *fmt, ...)
{
    vesa_set_color(vesa_dark_yellow);
    kernel_time();
    printf("WARN: ");

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    vesa_set_color(default_text_color);
    writec('\n');
}

void panic(char *msg)
{
    asm ("cli");
    vesa_set_color(vesa_dark_red);
    kernel_time();
    serial_write("\nPANIC: ");
    serial_write(msg);
    serial_write(" - System halted!\n");
    printf("PANIC: %s - System halted!\n", msg);
    halt_loop();
}

void assert(int x)
{
    if (x == 0) {
        panic("Assertion failed");
    }
}

void halt_loop()
{
    serial_write("\n!!! HALT !!!\n");
    loop:
    asm ("hlt");
    goto loop;
}

void v86(uint8_t code, regs16_t *regs)
{
    paging_disable();
    int32(code, regs);
    paging_enable();
}

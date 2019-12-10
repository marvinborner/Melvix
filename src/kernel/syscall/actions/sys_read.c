#include <stdint.h>
#include <kernel/lib/stdio.h>
#include <kernel/input/input.h>
#include <kernel/lib/lib.h>
#include <kernel/lib/string.h>
#include <kernel/io/io.h>

uint32_t sys_read(char *buf)
{
    keyboard_clear_buffer();
    keyboard_char_buffer = 0;
    while (keyboard_char_buffer != '\n') {
        getch();
    }
    memcpy(buf, keyboard_buffer, strlen(keyboard_buffer));
    return strlen(buf);
}

uint32_t sys_readc(char *ch)
{
    char buf = getch();
    ch = &buf;
    serial_put(*ch);
    return (uint32_t) ch;
}
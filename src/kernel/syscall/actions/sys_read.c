#include <stdint.h>
#include <kernel/lib/stdio.h>
#include <kernel/input/input.h>
#include <kernel/lib/lib.h>
#include <kernel/lib/string.h>

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

uint32_t sys_readc()
{
    return (uint32_t) getch();
}
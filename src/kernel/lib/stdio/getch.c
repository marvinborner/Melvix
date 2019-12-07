#include <kernel/input/input.h>
#include <kernel/lib/string.h>

char getch()
{
    keyboard_clear_buffer();
    while (strlen(keyboard_buffer) == 0) {}
    return keyboard_buffer[0];
}
#include <stdint-gcc.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/string.h>
#include <kernel/io/io.h>
#include <kernel/input/input.h>
#include <kernel/timer/timer.h>

uint32_t sys_read()
{
    serial_write("CALL\n");

    keyboard_clear_buffer();
    while (keyboard_buffer[strlen(keyboard_buffer) - 1] != '\n') {
        {
            timer_wait(1); // IDK why?
        }
    }
    serial_write(keyboard_buffer);
    serial_write("\n");
    return (uint32_t) keyboard_buffer;
}
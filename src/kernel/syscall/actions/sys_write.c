#include <stdint-gcc.h>
#include <mlibc/stdio.h>
#include <kernel/io/io.h>

uint32_t sys_write(char *buf, uint32_t count)
{
    serial_write("WRITE");
    for (uint32_t i = 0; i < count; i++)
        writec(*(buf++));
    return count;
}
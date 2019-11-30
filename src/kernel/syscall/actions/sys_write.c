#include <stdint-gcc.h>
#include <mlibc/stdio.h>

uint32_t sys_write(char *buf, uint32_t count)
{
    for (uint32_t i = 0; i < count; i++)
        writec(*(buf++));
    return count;
}
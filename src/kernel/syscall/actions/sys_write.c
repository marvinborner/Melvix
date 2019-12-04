#include <stdint-gcc.h>
#include <mlibc/stdio.h>

uint32_t sys_write(unsigned int fd, unsigned int buf, unsigned int count)
{
    for (uint32_t i = 0; i < count; i++)
        writec(*((char *) buf++));
    return count;
}
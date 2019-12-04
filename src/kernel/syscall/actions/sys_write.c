#include <stdint-gcc.h>
#include <mlibc/stdio.h>
#include <kernel/io/io.h>

uint32_t sys_write(unsigned int buf, unsigned int count)
{
    serial_write("\n");
    serial_write_dec(count);
    serial_write("WRITE: \n");
    serial_write((const char *) buf);
    return count;
}
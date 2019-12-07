#include <stdint-gcc.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/string.h>
#include <kernel/io/io.h>

uint32_t sys_write(unsigned int buf)
{
    serial_put(((char *) buf)[0]);
    printf((const char *) buf);
    return strlen((const char *) buf);
}
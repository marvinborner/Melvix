#include <stdint-gcc.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/string.h>
#include <kernel/io/io.h>

uint32_t sys_write(char *buf)
{
    printf(buf);
    return strlen((const char *) buf);
}
#include <stdint.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/string.h>
#include <kernel/io/io.h>

uint32_t sys_write(char *buf)
{
    printf(buf);
    return strlen((const char *) buf);
}

uint32_t sys_writec(char *ch)
{
    serial_write_hex(*ch);
    serial_write("\n\n");
    writec((char) *ch);
    return 0;
}
#include <stdint.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/string.h>

uint32_t sys_write(char *buf)
{
    printf(buf);
    return strlen((const char *) buf);
}

uint32_t sys_writec(char ch)
{
    writec(ch);
    return 0;
}
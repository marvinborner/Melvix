#include <stdint.h>
#include <kernel/lib/stdlib/liballoc.h>

uint32_t sys_free(uint32_t ptr)
{
    ufree((void *) ptr);
    return 0;
}
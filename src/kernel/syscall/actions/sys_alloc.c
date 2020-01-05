#include <stdint.h>
#include <kernel/lib/stdlib/liballoc.h>

uint32_t sys_alloc(uint32_t count)
{
    return (uint32_t) umalloc(count);
}
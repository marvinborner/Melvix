#include <stdint.h>

uint32_t sys_alloc(uint32_t count)
{
    return 0; // (uint32_t) umalloc(count);
}
#include <stdint.h>
#include <kernel/memory/kheap.h>

uint32_t sys_free(uint32_t ptr)
{
    kfree((void *) ptr);
    return 0;
}
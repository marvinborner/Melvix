#include <stdint.h>
#include <kernel/paging/paging.h>

uint32_t sys_paging_free(uint32_t virt, uint32_t count)
{
    paging_set_free(virt, count);
    return count;
}
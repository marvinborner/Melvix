#include <stdint.h>
#include <kernel/paging/paging.h>
#include <kernel/io/io.h>

uint32_t sys_paging_alloc(uint32_t count)
{
    uint32_t ptr = paging_alloc_pages((uint32_t) count);
    paging_set_user(ptr, count);
    return ptr;
}
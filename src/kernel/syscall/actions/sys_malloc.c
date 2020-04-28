#include <stdint.h>
#include <kernel/memory/alloc.h>

uint32_t sys_malloc(uint32_t count)
{
	return (uint32_t)umalloc(count);
}
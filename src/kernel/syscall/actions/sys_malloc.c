#include <stdint.h>
#include <memory/alloc.h>

u32 sys_malloc(u32 count)
{
	return (u32)umalloc(count);
}
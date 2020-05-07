#include <memory/alloc.h>
#include <stdint.h>

u32 sys_free(u32 ptr)
{
	ufree((void *)ptr);
	return 0;
}
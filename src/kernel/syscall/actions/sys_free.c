#include <stdint.h>
#include <memory/alloc.h>

u32 sys_free(u32 ptr)
{
	ufree((void *)ptr);
	return 0;
}
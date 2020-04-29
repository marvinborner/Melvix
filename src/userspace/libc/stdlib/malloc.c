#include <stdint.h>
#include <syscall.h>

void *malloc(u32 size)
{
	return (void *)syscall_malloc(size);
}
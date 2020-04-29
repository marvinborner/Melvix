#include <stdint.h>
#include <syscall.h>

void *malloc(u8 size)
{
	return (void *)syscall_malloc(size);
}
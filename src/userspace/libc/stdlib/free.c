#include <stdint.h>
#include <syscall.h>

void free(void *addr)
{
	syscall_free((u32)addr);
}
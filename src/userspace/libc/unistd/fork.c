#include <stdint.h>
#include <syscall.h>

u32 fork()
{
	return syscall_fork();
}
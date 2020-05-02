#include <stdint.h>
#include <syscall.h>

u32 exec(char *path)
{
	return syscall_exec(path);
}
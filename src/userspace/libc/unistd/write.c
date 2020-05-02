#include <stdint.h>
#include <syscall.h>

u32 sys_write(char *path, u32 offset, u32 count, char *buf)
{
	return syscall_write(path, offset, count, buf);
}
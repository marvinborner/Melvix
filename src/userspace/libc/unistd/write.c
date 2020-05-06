#include <stdint.h>
#include <syscall.h>

u32 write(char *path, u32 offset, u32 count, u8 *buf)
{
	return syscall_write(path, offset, count, buf);
}
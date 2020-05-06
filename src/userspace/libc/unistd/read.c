#include <stdint.h>
#include <syscall.h>

u32 read(char *path, u32 offset, u32 count, u8 *buf)
{
	return syscall_read(path, offset, count, buf);
}
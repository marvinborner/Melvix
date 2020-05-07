#include <fs/fs.h>
#include <stdint.h>

u32 sys_write(char *path, u32 offset, u32 count, u8 *buf)
{
	return write(path, offset, count, buf);
}
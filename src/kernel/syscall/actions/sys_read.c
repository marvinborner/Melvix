#include <stdint.h>
#include <fs/fs.h>

u32 sys_read(char *path, u32 offset, u32 count, u8 *buf)
{
	return read(path, offset, count, buf);
}
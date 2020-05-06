#include <stdint.h>
#include <kernel/fs/fs.h>

uint32_t sys_read(char *path, uint32_t offset, uint32_t count, uint8_t *buf)
{
	return read(path, offset, count, buf);
}
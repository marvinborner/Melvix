#include <stdint.h>
#include <kernel/fs/fs.h>

uint32_t sys_write(char *path, uint32_t offset, uint32_t count, uint8_t *buf)
{
	return write(path, offset, count, buf);
}
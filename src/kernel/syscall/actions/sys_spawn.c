#include <stdint.h>
#include <tasks/process.h>

u32 sys_spawn(char *path)
{
	return uspawn(path);
}
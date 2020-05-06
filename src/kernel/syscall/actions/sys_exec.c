#include <stdint.h>
#include <tasks/process.h>

u32 sys_exec(char *path)
{
	return uexec(path);
}
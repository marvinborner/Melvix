#include <stdint.h>
#include <kernel/tasks/process.h>

uint32_t sys_exec(char *path)
{
	return uexec(path);
}

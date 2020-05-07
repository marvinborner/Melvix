#include <stdint.h>
#include <tasks/process.h>

u32 sys_get_pid()
{
	return current_proc->pid;
}

#include <stdint.h>
#include <kernel/tasks/process.h>

uint32_t sys_get_pid()
{
	return current_proc->pid;
}
#include <stdint.h>
#include <kernel/tasks/process.h>

uint32_t sys_exit(uint32_t code)
{
	current_proc->state = PROC_ASLEEP;
	return code;
}
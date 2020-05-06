#include <stdint.h>
#include <tasks/process.h>

u32 sys_exit(u32 code)
{
	current_proc->state = PROC_ASLEEP;
	return code;
}
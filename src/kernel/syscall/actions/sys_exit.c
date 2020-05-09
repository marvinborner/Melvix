#include <stdint.h>
#include <tasks/process.h>

u32 sys_exit(u32 code)
{
	process_suspend(current_proc->pid);
	return code;
}
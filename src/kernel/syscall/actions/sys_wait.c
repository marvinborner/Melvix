#include <io/io.h>
#include <stdint.h>
#include <tasks/process.h>

u32 sys_wait(u32 pid, u32 *status, u32 options)
{
	if (pid > 0)
		return process_wait_pid(pid, status);
	return -1;
}
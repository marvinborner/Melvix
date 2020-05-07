#include <io/io.h>
#include <stdint.h>
#include <tasks/process.h>

u32 sys_wait(u32 pid, u32 *status, u32 options)
{
	sti();
	u32 ret;

	if (pid < 0) { // Wait for any process in gid to die
		ret = process_wait_gid(pid * -1, status);
	}

	if (pid == -1) { // Wait for any child to be killed
		ret = process_wait_gid(current_proc->pid, status);
	}

	if (pid == 0) { // Wait for siblings of this process to die
		ret = process_wait_gid(current_proc->gid, status);
	}

	if (pid > 0) { // Wait for pid to die
		ret = process_wait_pid(pid, status);
	}

	return ret;
}
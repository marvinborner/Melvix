#include <stdint.h>
#include <syscall.h>

u32 wait_pid(u32 pid, u32 *status, u32 options)
{
	return syscall_wait(pid, status, options);
}

u32 wait(u32 *status)
{
	return wait_pid(-1, status, 0);
}
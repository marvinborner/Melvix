#include <stdint.h>
#include <syscall.h>

u32 get_pid()
{
	return syscall_get_pid();
}
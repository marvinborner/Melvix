#include <stdint.h>
#include <syscall.h>

u32 spawn(char *path)
{
	return syscall_spawn(path);
}
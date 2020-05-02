#include <stdint.h>
#include <syscall.h>

void exit(u32 code)
{
	syscall_exit(code);
	while (1) {
	};
}
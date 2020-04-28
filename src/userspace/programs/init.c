#include <syscall.h>

void main()
{
	syscall_exec("/bin/sh");

	while (1) {
	};
}

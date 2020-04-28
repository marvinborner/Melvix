#include <syscall.h>

void main()
{
	syscall_write("\nHello from Userspace!\n");

	syscall_write("> ");

	syscall_halt();
}

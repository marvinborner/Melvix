#include <syscall.h>

void main()
{
	syscall_putch('\n');
	syscall_putch('>');
	syscall_putch(' ');

	while (1) {
		syscall_putch(syscall_getch());
	}

	syscall_halt();
}
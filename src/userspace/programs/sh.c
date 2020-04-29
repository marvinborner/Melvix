#include <syscall.h>
#include <stdio.h>

void main()
{
	printf("Test for printf! %d\n", 42);
	printf("[~] ");

	while (1) {
		putch(getch());
	}

	syscall_halt();
}
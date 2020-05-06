#include <stdio.h>
#include <unistd.h>
#include <syscall.h>
#include <gui.h>

void main()
{
	printf("[~] ");

	while (1) {
		putch(getch());
	}

	syscall_halt();
}
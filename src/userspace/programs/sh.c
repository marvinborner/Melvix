#include <stdio.h>
#include <unistd.h>
#include <syscall.h>
#include <gui.h>

void main()
{
	write("/dev/fb", 0, 5, "hallo");
	printf("[~] ");

	while (1) {
		putch(getch());
	}

	syscall_halt();
}
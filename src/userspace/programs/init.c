#include <stdio.h>
#include <syscall.h>
#include <gui.h>

void main()
{
	printf("Initializing userspace...\n");
	//gui_init();
	syscall_exec("/bin/sh");

	while (1) {
	};
}
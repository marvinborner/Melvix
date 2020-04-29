#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>
#include <gui.h>

void main()
{
	u32 *buf = malloc(4096);
	for (int i = 0; i < 4; i++)
		buf[i] = 42;
	syscall_halt();

	//printf("Initializing userspace...\n");
	//gui_init();
	//syscall_exec("/bin/sh");

	while (1) {
	};
}
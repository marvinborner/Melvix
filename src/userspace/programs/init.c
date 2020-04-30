#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>
#include <gui.h>

void main()
{
	gui_init();
	gui_screen_clear();
	printf("Initializing userspace...\n");
	syscall_exec("/bin/sh");

	while (1) {
	};
}
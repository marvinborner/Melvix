#include <stdio.h>
#include <stdlib.h>
#include <common.h>
#include <syscall.h>
#include <unistd.h>
#include <gui.h>

void test(u8 *data)
{
	syscall_halt();
}

void main()
{
	/* gui_init(); */
	/* gui_screen_clear(); */
	//printf("Initializing userspace...\n");
	syscall_map(MAP_KEYBOARD, (u8 *)&test);

	//syscall_exec("/bin/sh");

	while (1) {
	};
}
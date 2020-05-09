#include <common.h>
#include <gui.h>
#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>
#include <unistd.h>

void main()
{
	if (get_pid() != 1) {
		printf("Wrong PID!\n");
		exit(1);
	}

	// TODO: Fix page fault when mallocing
	printf("Initializing userspace...\n");

	// TODO: Find out, why processes change pid randomly
	// TODO: Fix occasional race conditions with cli/sti
	// TODO: Fix scheduler turning off at some point
	spawn("/bin/sh");
	printf("ok");

	while (1) {
		//printf("B");
	};
}
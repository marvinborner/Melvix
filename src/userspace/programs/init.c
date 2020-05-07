#include <stdio.h>
#include <stdlib.h>
#include <common.h>
#include <syscall.h>
#include <unistd.h>
#include <gui.h>

void main()
{
	if (get_pid() != 2) {
		printf("Wrong PID!\n");
		exit(1);
	}

	// TODO: Fix page fault when mallocing
	printf("Initializing userspace...\n");

	// TODO: Fix occasional race conditions with cli/sti
	// TODO: Fix scheduler turning off randomly..
	u32 x;
	u32 f = fork();
	if (f == 0) {
		printf("Waiting...\n");
		wait(&x);
	} else {
		printf("Executing...\n");
		exec("/bin/sh");
	}

	while (1) {
		//printf("B");
	};
}
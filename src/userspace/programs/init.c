#include <stdio.h>
#include <stdlib.h>
#include <common.h>
#include <syscall.h>
#include <unistd.h>
#include <gui.h>

void main()
{
	// TODO: Fix page fault when mallocing
	printf("Initializing userspace...\n");

	u32 x;
	u32 f = fork();
	if (f == 0)
		wait(&x);
	else
		exec("/bin/sh");

	while (1) {
	};
}
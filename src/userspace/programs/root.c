#include <stdio.h>
#include <unistd.h>
#include <syscall.h>

// This process only exists because it can't crash
void main()
{
	if (get_pid() != 1) {
		printf("Wrong PID!\n");
		exit(1);
	}

	exec("/bin/init");
	printf("The init process crashed!");
}
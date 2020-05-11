#include <common.h>
#include <gui.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <syscall.h>
#include <unistd.h>

void test(u8 *data)
{
	printf(".");
}

u32 cpu_flags()
{
	u32 flags;
	asm volatile("pushf\n"
		     "pop %0\n"
		     : "=rm"(flags)::"memory");
	return flags;
}

int interrupts_enabled()
{
	return (cpu_flags() & 0x200) == 0x200;
}

void main()
{
	/* if (get_pid() != 1) { */
	/* 	printf("Wrong PID!\n"); */
	/* 	exit(1); */
	/* } */

	if (interrupts_enabled())
		printf("INTs enabled :)\n");
	else
		printf("INTs disabled :(\n");
	printf("[~] ");

	/* while (1) { */
	/* 	putch(getch()); */
	/* } */
	//syscall_map(MAP_KEYBOARD, (u32)&test);

	/* syscall_halt(); */
	while (1) {
		//printf("A");
	};
}
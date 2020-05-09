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

void main()
{
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
// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <boot.h>
#include <conv.h>
#include <cpu.h>
#include <fs.h>
#include <keyboard.h>
#include <load.h>
#include <math.h>
#include <mem.h>
#include <mouse.h>
#include <serial.h>
#include <str.h>
#include <syscall.h>
#include <timer.h>

#define a_mag 0x55
#define b_mag 0x42
#define check(exp)                                                                                 \
	if (!(exp)) {                                                                              \
		printf("\x1B[31m[FAIL]\x1B[0m %s:%d: %s: Test '%s'\n", __FILE__, __LINE__,         \
		       __func__, #exp);                                                            \
	} else {                                                                                   \
		printf("\x1B[32m[PASS]\x1B[0m %s:%d: %s: Test '%s'\n", __FILE__, __LINE__,         \
		       __func__, #exp);                                                            \
	}

void test_all(struct vid_info *vid_info)
{
	// Serial connection
	serial_install();
	serial_print("\nConnected testing.\n");

	// Boot passed
	check(vid_info && vid_info->mode && vid_info->vbe);

	// Test malloc
	heap_init(0x00f00000);
	u32 *a = malloc(a_mag);
	u32 *b = malloc(b_mag);
	check(a[-1] == a_mag);
	check(a[a_mag] == b_mag);
	check(b[-1] == b_mag);

	// Test math
	check(pow(2, 3) == 8);
	check(pow(0, 3) == 0);
	check(pow(0, 0) == 1);

	// Test conv
	check(atoi("42") == 42);
	check(strcmp(htoa(0x42), "42") == 0);
	check(htoi("42") == 0x42);
	check(strcmp(itoa(42), "42") == 0);

	boot_passed = vid_info;

	// Install drivers
	interrupts_install();
	timer_install();
	keyboard_install();
	mouse_install();

	idle();
}

// MIT License, Copyright (c) 2020 Marvin Borner

#include <boot.h>
#include <conv.h>
#include <cpu.h>
#include <math.h>
#include <mem.h>
#include <serial.h>
#include <str.h>
#include <test.h>

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
	equals(a[-1], a_mag);
	equals(a[a_mag], b_mag);
	equals(b[-1], b_mag);

	// Test math
	equals(pow(2, 3), 8);
	equals(pow(0, 3), 0);
	equals(pow(0, 0), 1);

	// Test conv
	char buf[10] = { 0 };
	strcpy(buf, "42");
	equals(atoi(buf), 42);
	equals_str(htoa(0x42), "42");
	equals(htoi(buf), 0x42);
	equals_str(itoa(42), "42");
	equals_str(conv_base(42, buf, 0, 0), "");
	equals_str(conv_base(42, buf, 2, 0), "101010");
	/* equals_str(conv_base(424242, buf, 36, 0), "93ci"); // TODO: THIS */

	idle();
}

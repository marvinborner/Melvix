// MIT License, Copyright (c) 2020 Marvin Borner

#include <boot.h>
#include <conv.h>
#include <cpu.h>
#include <math.h>
#include <mem.h>
#include <serial.h>
#include <str.h>
#include <test.h>

void pass_or_fail(const char *file_name, int line_num, const char *func, const char *first,
		  const char *second, int success)
{
	printf("\x1B[%s\x1B[0m %s:%d: %s: %s == %s\n", success ? "32m[PASS]" : "31m[FAIL]",
	       file_name, line_num, func, first, second);
}

void test_malloc()
{
	heap_init(0x00f00000);
	u32 *a = malloc(a_mag);
	u32 *b = malloc(b_mag);
	equals(a[-1], a_mag);
	equals(a[a_mag], b_mag);
	equals(b[-1], b_mag);
}

void test_math()
{
	equals(pow(2, 3), 8);
	equals(pow(0, 3), 0);
	equals(pow(0, 0), 1);
}

void test_conv()
{
	char buf0[3] = { 0 };
	char buf1[3] = { 0 };
	char buf2[1] = { 0 };
	char buf3[7] = { 0 };
	char buf4[5] = { 0 };
	char buf5[2] = { 0 };
	strcpy(buf0, "42");
	strcpy(buf1, "42");
	equals(atoi(buf0), 42);
	equals_str(htoa(0x42), "42");
	equals(htoi(buf1), 0x42);
	equals_str(itoa(42), "42");
	equals_str(conv_base(42, buf2, 0, 0), "");
	equals_str(conv_base(42, buf3, 2, 0), "101010");
	equals_str(conv_base(424242, buf4, 36, 0), "93ci");
	equals_str(conv_base(0xffffffff, buf5, 10, 1), "-1");
}

void test_mem()
{
	char *str0 = "";
	char *str1 = "";
	char *str2 = "12345";
	char *str3 = "12345";
	char *str4 = "12354";
	equals(memcmp(str4, str2, strlen(str2)), 1);
	equals(memcmp(str2, str4, strlen(str2)), -1);
	equals(memcmp(str2, str3, strlen(str2)), 0);
	equals(memcmp(str0, str1, strlen(str0)), 0);
	equals(memcmp(NULL, NULL, 0), 0);

	char buf[6];
	equals_str(memcpy(buf, "hallo", 5), "hallo");

	char buf2[6];
	equals_str(memset(buf2, 'x', 5), "xxxxx");
}

void test_all(struct vid_info *vid_info)
{
	// Serial connection
	serial_install();
	serial_print("\nConnected testing.\n");

	// Boot passed
	check(vid_info && vid_info->mode && vid_info->vbe);

	test_malloc();
	test_math();
	test_conv();
	test_mem();

	idle();
}

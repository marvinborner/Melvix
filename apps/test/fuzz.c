// MIT License, Copyright (c) 2021 Marvin Borner

#include "test.h"

#include <def.h>
#include <print.h>
#include <rand.h>
#include <sys.h>

#define FUZZ_COUNT 1000

static res syscall(enum sys num, int d1, int d2, int d3, int d4, int d5)
{
	int a;
	__asm__ volatile("int $0x80"
			 : "=a"(a)
			 : "0"(num), "b"((int)d1), "c"((int)d2), "d"((int)d3), "S"((int)d4),
			   "D"((int)d5));
	return a;
}

static u8 deadly_call(enum sys num)
{
	return num == SYS_EXIT;
}

static enum sys random_call(void)
{
	u32 num;
	do {
		num = rand_range(SYS_MIN, SYS_MAX);
	} while (deadly_call(num));
	return num;
}

void fuzz(void)
{
	u32 cnt = FUZZ_COUNT;
	while (cnt) {
		enum sys num = random_call();
		u32 d1 = rand();
		u32 d2 = rand();
		u32 d3 = rand();
		u32 d4 = rand();
		u32 d5 = rand();

		log("%d\n", syscall(num, d1, d2, d3, d4, d5));

		cnt--;
	}

	log("Fuzzer: OK!\n");
}

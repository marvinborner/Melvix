// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <def.h>
#include <rand.h>
#include <sys.h>

extern u32 __stack_chk_guard;
u32 __stack_chk_guard;

int main(int, char **);

int _start(int argc, char **argv);
int _start(int argc, char **argv)
{
	struct timer timer = { 0 };
	assert(dev_read(DEV_TIMER, &timer, 0, sizeof(timer)) == sizeof(timer));
	srand(timer.rtc + timer.time);
	__stack_chk_guard = rand();

	exit(main(argc, argv));

	return 1;
}

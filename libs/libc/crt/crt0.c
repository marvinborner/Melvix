// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <def.h>
#include <random.h>
#include <sys.h>

#ifdef USER

extern u32 __stack_chk_guard;
u32 __stack_chk_guard;

int main(int, char **);

int _start(int argc, char **argv);
int _start(int argc, char **argv)
{
	u32 stamp = 0;
	assert(read("/dev/rtc", &stamp, 0, sizeof(stamp)) == sizeof(stamp) && stamp);
	srand(stamp);
	__stack_chk_guard = rand();

	exit(main(argc, argv));

	return 1;
}

#endif

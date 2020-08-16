// MIT License, Copyright (c) 2020 Marvin Borner
// Syscall implementation

#include <arg.h>
#include <sys.h>

/**
 * Definitions
 */

int sys0(enum sys num)
{
	int a;
	__asm__ volatile("int $0x80" : "=a"(a) : "0"(num));
	return a;
}

int sys1(enum sys num, int d1)
{
	int a;
	__asm__ volatile("int $0x80" : "=a"(a) : "0"(num), "b"((int)d1));
	return a;
}

int sys2(enum sys num, int d1, int d2)
{
	int a;
	__asm__ volatile("int $0x80" : "=a"(a) : "0"(num), "b"((int)d1), "c"((int)d2));
	return a;
}

int sys3(enum sys num, int d1, int d2, int d3)
{
	int a;
	__asm__ volatile("int $0x80"
			 : "=a"(a)
			 : "0"(num), "b"((int)d1), "c"((int)d2), "d"((int)d3));
	return a;
}

int sys4(enum sys num, int d1, int d2, int d3, int d4)
{
	int a;
	__asm__ volatile("int $0x80"
			 : "=a"(a)
			 : "0"(num), "b"((int)d1), "c"((int)d2), "d"((int)d3), "S"((int)d4));
	return a;
}

int sys5(enum sys num, int d1, int d2, int d3, int d4, int d5)
{
	int a;
	__asm__ volatile("int $0x80"
			 : "=a"(a)
			 : "0"(num), "b"((int)d1), "c"((int)d2), "d"((int)d3), "S"((int)d4),
			   "D"((int)d5));
	return a;
}

#include <print.h>
int sysv(enum sys num, ...)
{
	va_list ap;
	int args[5];

	va_start(ap, num);
	for (int i = 0; i < 5; i++)
		args[i] = va_arg(ap, int);
	va_end(ap);

	return sys5(num, args[0], args[1], args[2], args[3], args[4]);
}

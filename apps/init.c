// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>
#include <print.h>
#include <str.h>

void main()
{
	print("Init loaded\n");
	__asm__ volatile("int $0x80");
	while (1) {
		print("b");
	};
}

// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>
#include <print.h>
#include <sys.h>

void main()
{
	print("\nA loaded!\n");
	sys0(SYS_HALT);
	while (1) {
		print("a");
	}
}

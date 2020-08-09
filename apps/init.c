// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>
#include <print.h>
#include <sys.h>

void main()
{
	print("Init loaded.\n");
	sys1(SYS_EXEC, (int)"/a");
	while (1) {
		print("b");
	};
}

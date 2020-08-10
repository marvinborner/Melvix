// MIT License, Copyright (c) 2020 Marvin Borner

#include <conv.h>
#include <def.h>
#include <mem.h>
#include <print.h>
#include <sys.h>

void main()
{
	print("Init loaded.\n");

	char *buf = malloc(10);
	conv_base(42, buf, 8, 0);
	printf("\n----\nTEST: %s\n----\n", buf);
	/* printf("%x %d %b\n ABC %s", 42, 42, 42, "BAUM"); */
	sys0(SYS_LOOP);
	/* sys1(SYS_EXEC, (int)"/a"); */
	while (1) {
		print("b");
	};
}

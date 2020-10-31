// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>
#include <print.h>
#include <sys.h>

int main(int argc, char **argv)
{
	(void)argc;
	/* printf("ARGC: %d\n", argc); */
	/* printf("[%s loaded]\n", argv[0]); */

	int wm = exec("/wm", "wm", argv[1], NULL);
	int exec = exec("/exec", "test", NULL);

	return wm + exec;
}

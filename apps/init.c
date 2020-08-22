// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>
#include <print.h>
#include <sys.h>

int main(int argc, char **argv)
{
	printf("ARGC: %d\n", argc);
	printf("[%s loaded]\n", argv[0]);

	int wm = exec("/wm", "wm", argv[1], NULL);
	int test = exec("/test", "test", NULL);
	return wm + test;
}

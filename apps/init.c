// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>
#include <net.h>
#include <print.h>
#include <str.h>
#include <sys.h>

#include <cpu.h>

int main(int argc, char **argv)
{
	UNUSED(argc);
	UNUSED(argv);

	int wm = exec("/bin/wm", "wm", NULL);
	int test = exec("/bin/window", "test", NULL);

	return wm + test;
}

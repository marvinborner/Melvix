// MIT License, Copyright (c) 2020 Marvin Borner

#include <conv.h>
#include <def.h>
#include <gui.h>
#include <mem.h>
#include <print.h>
#include <sys.h>
#include <vesa.h>

int main(int argc, char **argv)
{
	(void)argc;

	printf("ARGC: %d\n", argc);
	printf("ARGV: %x\n", argv);
	printf("%s loaded.\n", argv[0]);

	exec("/wm", "wm", argv[1], NULL);
	return 0;
}

// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>
#include <net.h>
#include <print.h>
#include <str.h>
#include <sys.h>

int main(int argc, char **argv)
{
	log("Loaded!\n");
	while (1) {
	};
	(void)argc;
	log("%s loaded\n", argv[0]);

	int wm = exec("/bin/wm", "wm", argv[1], NULL);
	int test = exec("/bin/window", "test", NULL);

	return wm + test;
}

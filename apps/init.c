// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <def.h>
#include <sys.h>

int main(int argc, char **argv)
{
	UNUSED(argc);
	UNUSED(argv);
	log("Arrived!\n");

	/* assert(exec("/bin/wm", "wm", NULL) == 0); */
	/* assert(exec("/bin/window", "test", NULL) == 0); */

	return 0;
}

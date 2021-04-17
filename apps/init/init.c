// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <def.h>
#include <sys.h>

int main(int argc, char **argv)
{
	UNUSED(argc);
	UNUSED(argv);

	assert(exec("wm", NULL) == EOK);

	return 0;
}

// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>
#include <fs.h>
#include <load.h>
#include <print.h>

void bin_load(char *path)
{
	char *data = read_file(path);

	void (*entry)();
	*(void **)(&entry) = data;

	entry();
}

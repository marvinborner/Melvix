// MIT License, Copyright (c) 2020 Marvin Borner

#include <conv.h>
#include <def.h>
#include <mem.h>
#include <print.h>
#include <sys.h>

void main()
{
	print("Init loaded.\n");

	exec("/a");
	exec("/b");
	exit();
}

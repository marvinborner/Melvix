// MIT License, Copyright (c) 2020 Marvin Borner

#include <conv.h>
#include <def.h>
#include <gui.h>
#include <mem.h>
#include <print.h>
#include <sys.h>
#include <vesa.h>

void main(struct vbe *vbe)
{
	print("Init loaded.\n");
	int a = exec("/wm", vbe);

	if (a)
		exit();
}

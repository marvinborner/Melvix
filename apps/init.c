// MIT License, Copyright (c) 2020 Marvin Borner

#include <conv.h>
#include <def.h>
#include <mem.h>
#include <print.h>
#include <sys.h>
#include <vesa.h>

void main(struct vbe *vbe)
{
	print("Init loaded.\n");
	printf("VBE: %dx%d\n", vbe->width, vbe->height);

	/* exec("/a"); */
	/* exec("/b"); */
	exit();
}

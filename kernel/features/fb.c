// MIT License, Copyright (c) 2021 Marvin Borner

#include <def.h>
#include <drivers/bga.h>
#include <drivers/vbe.h>
#include <fb.h>
#include <multiboot.h>
#include <print.h>

#define FB_SIZE (generic->height * generic->pitch)

u32 fb_map_buffer(struct page_dir *dir, struct fb_generic *generic)
{
	struct memory_range r =
		virtual_alloc(dir, memory_range_around((u32)generic->fb, FB_SIZE), MEMORY_USER);
	return r.base;
}

CLEAR void fb_protect(struct fb_generic *generic)
{
	physical_set_used(memory_range_around((u32)generic->fb, FB_SIZE));
}

CLEAR void fb_install(void)
{
	if (bga_available())
		bga_install();
	else if (multiboot_vbe())
		vbe_install(multiboot_vbe());
	else
		panic("No framebuffer driver found!\n");
}

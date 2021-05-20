// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef FB_H
#define FB_H

#include <def.h>
#include <mm.h>
#include <sys.h>

u32 fb_map_buffer(struct page_dir *dir, struct fb_generic *generic);
void fb_protect(struct fb_generic *generic);
void fb_install(void);

#endif

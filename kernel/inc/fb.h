// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef FB_H
#define FB_H

#include <boot.h>
#include <mm.h>

void fb_map_buffer(struct page_dir *dir, struct vid_info *boot);
void fb_install(struct vid_info *boot);

#endif

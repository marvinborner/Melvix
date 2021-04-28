// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef FB_H
#define FB_H

#include <boot.h>
#include <mm.h>

void fb_map_buffer(struct page_dir *dir, struct vid_info *boot) NONNULL;
void fb_install(void) NONNULL;

#endif

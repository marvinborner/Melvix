// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef GUI_H
#define GUI_H

#include <def.h>
#include <gfx.h>

struct gui_window {
	u32 id;
	struct context *ctx;
	vec2 *pos;
};

s32 gui_new_window(struct gui_window *win);
s32 gui_redraw_window(u32 id);

#endif

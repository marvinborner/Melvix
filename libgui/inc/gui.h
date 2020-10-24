// MIT License, Copyright (c) 2020 Marvin Borner
// Mostly GFX function wrappers

#ifndef GUI_H
#define GUI_H

#include <def.h>
#include <gfx.h>

#define MAX_CHILDS 100

struct element {
	struct context *ctx;
};

struct window {
	const char *title;
	struct element *childs[MAX_CHILDS];
	struct context *ctx;
};

// TODO: Remove window return (internal)
struct window *gui_init(const char *title, u32 width, u32 height);

#endif

// MIT License, Copyright (c) 2020 Marvin Borner
// Mostly GFX function wrappers

#include <def.h>
#include <gfx.h>
#include <gui.h>
#include <mem.h>

#define MAX_WINDOWS 10

u32 window_count = 0;
static struct window windows[MAX_WINDOWS] = { 0 };

struct window *new_window(const char *title, int x, int y, u32 width, u32 height, int flags)
{
	if (window_count + 1 >= MAX_WINDOWS)
		return NULL;

	struct window *win = &windows[window_count + 1];
	win->ctx = malloc(sizeof(*win->ctx));
	win->ctx->x = x > 0 ? x : 50;
	win->ctx->y = y > 0 ? y : 50;
	win->ctx->width = width > 0 ? width : 600;
	win->ctx->height = height > 0 ? height : 400;
	win->ctx->flags = flags;
	win->title = title;
	gfx_new_ctx(windows[window_count + 1].ctx);

	if (!win->ctx->fb)
		return NULL;

	window_count++;

	return win;
}

struct window *gui_init(const char *title, u32 width, u32 height)
{
	if (window_count != 0)
		return NULL;

	struct window *win = new_window(title, 0, 0, width, height, WF_DEFAULT);
	if (!win)
		return NULL;

	gfx_fill(win->ctx, COLOR_BG);
	gfx_init("/font/spleen-12x24.psfu");

	return win;
}

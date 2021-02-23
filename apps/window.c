// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <gui.h>
#include <print.h>

int main()
{
	struct gui_window win = { 0 };
	assert(gui_new_window(&win) > 0);
	gfx_fill(win.ctx, COLOR_GREEN);
	assert(gui_redraw_window(win.id) > 0);
	log("%d\n", win.ctx->size.x);
	return 0;
}

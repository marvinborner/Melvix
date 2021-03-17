// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <gui.h>
#include <print.h>

int main(void)
{
	struct gui_window win = { 0 };
	assert(gui_new_window(&win) > 0);
	return 0;
#if 0
	gfx_fill(win.ctx, COLOR_GREEN);
	// Professional testing
	for (int i = 0; i < 12; i++) {
		gfx_write(win.ctx, vec2(0, i * gfx_font_height(FONT_32)), FONT_32,
			  0xff000000 + (i * 0xaf << i), "Hallo, wie geht es Ihnen denn heute?!");
	}
	assert(gui_redraw_window(win.id) > 0);
	log("%d\n", win.ctx->size.x);
#endif
	return 0;
}

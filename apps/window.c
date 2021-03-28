// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <libgui/gui.h>
#include <print.h>

static void mousemove(vec2 pos)
{
	log("%d %d\n", pos.x, pos.y);
}

int main(void)
{
	u32 win;
	assert((win = gui_new_window()) > 0);

	u32 main;
	assert((main = gui_new_widget(win, gui_window_size(win), vec2(0, 0))) > 0);

	assert(gui_fill(win, main, COLOR_BLACK) == EOK);
	assert(gui_redraw_widget(win, main) == EOK);

	assert(gui_listen_widget(win, main, GUI_LISTEN_MOUSEMOVE, (u32)mousemove) == EOK);

	gui_loop();
	return 0;
}

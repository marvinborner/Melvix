// MIT License, Copyright (c) 2021 Marvin Borner

#include <def.h>
#include <libgui/gui.h>

#define SCALING_FACTOR 0.1

static vec2 size = { 800, 800 };
static const char *path;

static void mousemove(struct gui_event_mouse *event)
{
	if (event->scroll) {
		size = vec2_mul(size, event->scroll < 0 ? SCALING_FACTOR + 1 : 1 - SCALING_FACTOR);
		gui_fill(event->win, event->widget, GUI_LAYER_FG, COLOR_WHITE);
		gui_draw_image(event->win, event->widget, GUI_LAYER_FG, vec2(0, 0), size, path);
		gui_redraw_widget(event->win, event->widget);
	}
}

int main(int argc, char *argv[])
{
	UNUSED(argc);
	UNUSED(argv);

	path = "/res/test.png";

	u32 win = gui_new_custom_window(APPNAME, vec2(0, 0), size);
	u32 canvas = gui_new_widget(win, GUI_MAIN, vec2(0, 0), size);
	gui_fill(win, canvas, GUI_LAYER_BG, COLOR_WHITE);
	gui_listen_widget(win, canvas, GUI_LISTEN_MOUSEMOVE, (u32)mousemove);

	gui_redraw_window(win);
	gui_loop();

	return 0;
}

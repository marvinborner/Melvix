// MIT License, Copyright (c) 2021 Marvin Borner

#include <libgui/gui.h>

#define COLOR_SIZE 50
#define TOOLBAR_MARGIN 2

static const u32 colors[] = { COLOR_BLACK, COLOR_RED,	  COLOR_GREEN, COLOR_YELLOW,
			      COLOR_BLUE,  COLOR_MAGENTA, COLOR_CYAN,  COLOR_WHITE };
static u32 current_color = COLOR_BLACK;
static u32 size = 5;

static void mousemove(struct gui_event_mouse *event)
{
	static vec2 last = { 0 };

	if (event->scroll && !(event->scroll == -1 && size <= 1))
		size += event->scroll;

	if (event->but.left) {
		gui_draw_line(event->win, event->widget, GUI_LAYER_FG, last, event->pos, size,
			      current_color);
		gui_redraw_widget(event->win, event->widget);
	}
	last = event->pos;
}

static void color_click(struct gui_event_mouse *event)
{
	current_color = colors[event->widget - 2];
}

int main(void)
{
	u32 win;
	gui_new_window(&win);
	vec2 win_size = gui_window_size(win);

	u32 toolbar = gui_new_widget(win, GUI_MAIN, vec2(0, 0), vec2(win_size.x, COLOR_SIZE));
	gui_fill(win, toolbar, GUI_LAYER_BG, COLOR_WHITE);

	u32 color_count = COUNT(colors);
	for (u32 i = 0; i < color_count; i++) {
		u32 color = gui_new_widget(win, toolbar, vec2(i * (COLOR_SIZE + TOOLBAR_MARGIN), 0),
					   vec2(COLOR_SIZE, COLOR_SIZE));
		gui_fill(win, color, GUI_LAYER_FG, colors[i]);
		gui_draw_border(win, color, GUI_LAYER_FG, 2, COLOR_BLACK);
		gui_listen_widget(win, color, GUI_LISTEN_MOUSECLICK, (u32)color_click);
	}

	u32 canvas = gui_new_widget(win, GUI_MAIN, vec2(0, COLOR_SIZE),
				    vec2(win_size.x, win_size.y - COLOR_SIZE));
	gui_fill(win, canvas, GUI_LAYER_BG, COLOR_WHITE);
	gui_listen_widget(win, canvas, GUI_LISTEN_MOUSEMOVE, (u32)mousemove);

	gui_redraw_window(win);
	gui_loop();
	return 0;
}

// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <libgui/gui.h>
#include <print.h>

#define SIZE 8
#define TILE 24
#define WHITE_STARTS 1

typedef u32 board[SIZE][SIZE];

static u32 win = 0; // Window
static board tiles = { 0 }; // Matrix

static void mouseclick(u32 widget_id, vec2 pos)
{
	UNUSED(pos);
	/* log("%d: %d %d\n", widget_id, pos.x, pos.y); */

	u32 x = widget_id / SIZE;
	u32 y = (widget_id % SIZE) - 1;
	u32 widget = tiles[x][y];
	assert(gui_fill(win, widget, COLOR_MAGENTA) == EOK);
	gui_redraw_widget(win, widget);
}

static void create_board(void)
{
	u32 widget;
	for (u8 x = 0; x < 8; x++) {
		for (u8 y = 0; y < 8; y++) {
			widget = gui_new_widget(win, vec2(TILE, TILE), vec2(TILE * x, TILE * y));
			assert(widget > 0);
			tiles[x][y] = widget;

			u8 colored = (x + y + 1) % 2 == 0;
#if !WHITE_STARTS
			colored = !colored;
#endif

			assert(gui_fill(win, widget, colored ? COLOR_BLACK : COLOR_WHITE) == EOK);
			assert(gui_listen_widget(win, widget, GUI_LISTEN_MOUSECLICK,
						 (u32)mouseclick) == EOK);
		}
	}

	assert(gui_redraw_window(win) == EOK);
}

int main(void)
{
	assert((win = gui_new_window()) > 0);
	create_board();
	gui_loop();
	return 0;
}

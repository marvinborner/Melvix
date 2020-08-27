// MIT License, Copyright (c) 2020 Marvin Borner

#include <conv.h>
#include <def.h>
#include <gui.h>
#include <print.h>
#include <str.h>

int main()
{
	print("[test loaded]\n");

	struct window *win = gui_new_window();

	gui_fill(win, BG_COLOR);
	gui_border(win, FG_COLOR, 2);

	gui_init("/font/spleen-12x24.psfu");
	char *hello = "Hello, world!";
	gui_write(win, win->width / 2 - (strlen(hello) * 12) / 2, 5, FG_COLOR, hello);

	while (1) {
		yield();
	}
	return 0;
}

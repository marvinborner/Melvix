// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>
#include <gui.h>
#include <print.h>

int main()
{
	print("[test loaded]\n");

	struct window *win = gui_new_window();
	u32 color[3] = { 0xff, 0xff, 0xff };
	gui_fill(win, color);

	while (1) {
		yield();
	}
	return 0;
}

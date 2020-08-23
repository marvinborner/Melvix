// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>
#include <gui.h>
#include <print.h>

int main()
{
	print("[test loaded]\n");

	struct window *win = gui_new_window();
	u32 color[3] = { 0xff, 0, 0 };
	gui_fill(win, color);

	u32 last_time = 0;
	while (1) {
		u32 current_time = time();
		if (current_time - last_time > 1000 / 60) { // 60Hz
			win->x += 10;
		};
		last_time = current_time;
		yield();
	}
	return 0;
}

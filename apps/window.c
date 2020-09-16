// MIT License, Copyright (c) 2020 Marvin Borner

#include <conv.h>
#include <def.h>
#include <gui.h>
#include <input.h>
#include <print.h>
#include <str.h>

int main()
{
	print("[test window loaded]\n");

	struct window win = { 0 };
	win.height = 200;
	win.width = 300;
	win.x = 50;
	win.y = 50;
	gui_new_window(&win);

	gui_fill(&win, COLOR_BG);
	gui_border(&win, COLOR_FG, 2);

	gui_init("/font/spleen-12x24.psfu");
	char *hello = "Hello, world!";
	gui_write(&win, win.width / 2 - (strlen(hello) * 12) / 2, 5, COLOR_GREEN, hello);

	struct message *msg;
	int char_x = 0;
	int char_y = 1;
	while (1) {
		if (!(msg = msg_receive())) {
			yield();
			continue;
		}

		switch (msg->type) {
		case WM_KEYBOARD: {
			struct msg_keyboard *event = msg->data;
			if (!event->press)
				break;
			gui_write_char(&win, 12 * char_x++, 24 * char_y + 5, COLOR_CYAN, event->ch);
			break;
		}
		default:
			break;
		}
	}
	return 0;
}

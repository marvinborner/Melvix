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

	struct window *win = gui_new_window(WF_DEFAULT);

	gui_fill(win, BG_COLOR);
	gui_border(win, FG_COLOR, 2);

	gui_init("/font/spleen-12x24.psfu");
	char *hello = "Hello, world!";
	gui_write(win, win->width / 2 - (strlen(hello) * 12) / 2, 5, FG_COLOR, hello);
	event_register(EVENT_KEYBOARD);

	struct message *msg;
	int char_x = 0;
	int char_y = 1;
	while (1) {
		if (!(msg = msg_receive())) {
			yield();
			continue;
		}
		switch (msg->type) {
		case EVENT_KEYBOARD: {
			struct event_keyboard *event = msg->data;

			if (event->magic != KEYBOARD_MAGIC)
				break;

			if (!event->press)
				break;

			int key = event->scancode;
			if (key == KEY_ENTER) {
				char_x = 0;
				char_y++;
			} else if (KEY_ALPHABETIC(key)) {
				gui_write_char(win, 12 * char_x++, 24 * char_y + 5, FG_COLOR, 'a');
			}

			break;
		}
		default:
			break;
		}
		yield();
	}
	return 0;
}

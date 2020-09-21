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
	win.height = 400;
	win.width = 600;
	win.x = 50;
	win.y = 50;
	gui_new_window(&win);

	gui_fill(&win, COLOR_BG);
	/* gui_border(&win, COLOR_FG, 2); */

	gui_init("/font/ter-p18n.psf");
	int font_height = gui_font_height();
	int font_width = gui_font_width();

	char *hello = "Hello, world!";
	gui_write(&win, win.width / 2 - (strlen(hello) * font_width) / 2, 0, COLOR_GREEN, hello);

	struct message *msg;
	int char_x = 0;
	int char_y = 1;
	while (1) {
		if (!(msg = msg_receive())) {
			yield();
			continue;
		}

		// TODO: Export to text widget or sth
		switch (msg->type) {
		case WM_KEYBOARD: {
			struct msg_keyboard *event = msg->data;
			char ch = event->ch;
			if (!event->press)
				break;

			if (ch == '\n') {
				char_x = 0;
				char_y++;
			} else if (ch == '\t') {
				char_x += 8;
			} else if (ch == '\b') {
				if (char_x > 0) {
					char_x--;
					gui_draw_rectangle(&win, font_width * char_x,
							   font_height * char_y,
							   font_width * (char_x + 1),
							   font_height * (char_y + 1), COLOR_BG);
				}
			} else if (ch == ' ' && event->scancode == KEY_SPACE) {
				char_x++;
			} else if (ch != ' ' && ch != '\0') {
				gui_write_char(&win, font_width * char_x++, font_height * char_y,
					       COLOR_CYAN, ch);
			}
			break;
		}
		default:
			break;
		}
	}
	return 0;
}

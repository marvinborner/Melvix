// MIT License, Copyright (c) 2020 Marvin Borner

#include <conv.h>
#include <def.h>
#include <gfx.h>
#include <gui.h>
#include <input.h>
#include <print.h>
#include <str.h>

int main()
{
	print("[test context loaded]\n");

	struct window *win = gui_init("test", 0, 0);
	struct context *ctx = win->ctx;

	int font_height = gfx_font_height();
	int font_width = gfx_font_width();

	char *hello = "Hello, world!";
	gfx_write(ctx, ctx->width / 2 - (strlen(hello) * font_width) / 2, 0, COLOR_GREEN, hello);

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

			if (char_x * font_width >= (int)ctx->width) {
				char_y++;
				char_x = 0;
			}

			if (ch == '\n') {
				char_x = 0;
				char_y++;
			} else if (ch == '\t') {
				char_x += 8;
			} else if (ch == '\b') {
				if (char_x > 0) {
					char_x--;
					gfx_draw_rectangle(ctx, font_width * char_x,
							   font_height * char_y,
							   font_width * (char_x + 1),
							   font_height * (char_y + 1), COLOR_BG);
				}
			} else if (ch == ' ' && event->scancode == KEY_SPACE) {
				char_x++;
			} else if (ch != ' ' && ch != '\0') {
				gfx_write_char(ctx, font_width * char_x++, font_height * char_y,
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

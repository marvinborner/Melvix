// MIT License, Copyright (c) 2020 Marvin Borner

#include <conv.h>
#include <def.h>
#include <gfx.h>
#include <gui.h>
#include <input.h>
#include <print.h>
#include <str.h>

void on_click()
{
	print("CLICK!\n");
}

int main()
{
	/* print("[test context loaded]\n"); */

	struct element *root = gui_init("test", 600, 400, COLOR_BG);
	struct element *container =
		gui_add_container(root, 0, 0, root->ctx->width / 2, root->ctx->height, COLOR_RED);
	struct element *button = gui_add_button(container, 10, 10, FONT_24, strdup("Button"),
						COLOR_WHITE, COLOR_BLACK);
	struct element *text_input =
		gui_add_text_input(container, 10, 50, 200, FONT_24, COLOR_WHITE, COLOR_BLACK);
	(void)text_input;

	button->event.on_click = on_click;

	gui_event_loop(root);

	return 0;
}

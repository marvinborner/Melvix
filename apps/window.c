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

	struct element *root = gui_init("test", 600, 400);
	struct element *container =
		gui_add_container(root, 100, 0, root->ctx->width / 2, root->ctx->height, COLOR_RED);
	struct element_button *button =
		gui_add_button(container, 10, 10, FONT_24, "Baum!", COLOR_WHITE, COLOR_BLACK)->data;

	button->on_click = on_click;

	gui_event_loop(root);

	return 0;
}

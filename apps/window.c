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
	print("[test context loaded]\n");

	struct element *container = gui_init("test", 0, 0);
	struct element_button *button =
		gui_add_button(container, 10, 10, FONT_24, "Baum!", COLOR_WHITE, COLOR_BLACK);
	gfx_border(container->ctx, COLOR_FG, 2);

	button->on_click = on_click;

	gui_event_loop(container);

	return 0;
}

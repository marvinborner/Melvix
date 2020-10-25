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

	struct element *container = gui_init("test", 0, 0);
	gui_add_button(container, 10, 10, 100, 20, "hallo", COLOR_RED);

	struct message *msg;
	while (1) {
		if (!(msg = msg_receive())) {
			yield();
			continue;
		}
	}
	return 0;
}

// MIT License, Copyright (c) 2020 Marvin Borner

#include <conv.h>
#include <def.h>
#include <gui.h>
#include <print.h>
#include <str.h>

int main()
{
	print("[test loaded]\n");

	struct window *win = gui_new_window();

	const u32 background[3] = { 0x28, 0x2c, 0x34 };
	gui_fill(win, background);
	const u32 border[3] = { 0xab, 0xb2, 0xbf };
	gui_border(win, border, 2);
	const u32 text[3] = { 0xab, 0xb2, 0xbf };

	gui_init("/font/spleen-12x24.psfu");
	char *hello = "Hello, world!";
	gui_write(win, win->width / 2 - (strlen(hello) * 12) / 2, 5, text, hello);

	while (1) {
		yield();
	}
	return 0;
}

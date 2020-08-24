// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>
#include <gui.h>
#include <input.h>
#include <list.h>
#include <mem.h>
#include <print.h>
#include <random.h>
#include <sys.h>
#include <vesa.h>

struct vbe *vbe;
struct window *direct; // Direct video memory window
struct window *root; // Root window (wallpaper etc.)
struct window *exchange; // Exchange buffer
struct list *windows;

static struct window *new_window(int x, int y, u16 width, u16 height)
{
	struct window *win = malloc(sizeof(*win));
	win->x = x;
	win->y = y;
	win->width = width;
	win->height = height;
	win->bpp = vbe->bpp;
	win->pitch = win->width * (win->bpp >> 3);
	win->fb = malloc(height * win->pitch);
	return win;
}

int main(int argc, char **argv)
{
	(void)argc;
	vbe = (struct vbe *)argv[1];
	printf("VBE: %dx%d\n", vbe->width, vbe->height);

	const u32 color[3] = { 0, 0, 0 };
	vesa_fill(vbe, color);
	gui_init("/font/spleen-16x32.psfu");

	windows = list_new();
	root = new_window(0, 0, vbe->width, vbe->height);
	exchange = new_window(0, 0, vbe->width, vbe->height);
	direct = malloc(sizeof(*direct));
	memcpy(direct, root, sizeof(*direct));
	direct->fb = vbe->fb;
	list_add(windows, root);

	const u32 background[3] = { 0x0, 0x0, 0x0 };
	gui_fill(root, background);
	// TODO: Fix wallpaper
	/* gui_load_wallpaper(root, "/wall.bmp"); */

	event_register(EVENT_KEYBOARD);

	struct message *msg;
	while (1) {
		if (windows->head && windows->head->data) {
			struct node *iterator = windows->head;
			do {
				struct window *win = iterator->data;
				gui_win_on_win(win, exchange, win->x, win->y);
			} while ((iterator = iterator->next) != NULL);
			gui_win_on_win(exchange, direct, 0, 0);
		}

		if (!(msg = msg_receive())) {
			yield();
			continue;
		}

		switch (msg->type) {
		case MSG_NEW_WINDOW:
			printf("New window for pid %d\n", msg->src);
			struct window *win = new_window(0, 0, 200, 200);
			msg_send(msg->src, MSG_NEW_WINDOW, win);
			list_add(windows, win);
			break;
		case EVENT_KEYBOARD:
			printf("Keypress %d!\n", msg->data);
			break;
		default:
			printf("Unknown WM request %d from pid %d", msg->type, msg->src);
		}
	};

	return 0;
}

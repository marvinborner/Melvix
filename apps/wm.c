// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <cpu.h>
#include <def.h>
#include <gui.h>
#include <input.h>
#include <list.h>
#include <mem.h>
#include <print.h>
#include <random.h>
#include <sys.h>
#include <vesa.h>

static struct vbe *vbe;
static struct window *direct; // Direct video memory window
static struct window *root; // Root window (wallpaper etc.)
static struct window *exchange; // Exchange buffer
static struct window *focused; // The focused window
static struct window *cursor; // Cursor bitmap window
static struct list *windows; // List of all windows

static int mouse_x = 0;
static int mouse_y = 0;

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

static void redraw_all()
{
	if (windows->head && windows->head->data) {
		struct node *iterator = windows->head;
		do {
			struct window *win = iterator->data;
			gui_win_on_win(exchange, win, win->x, win->y);
		} while ((iterator = iterator->next) != NULL);
		memcpy(direct->fb, exchange->fb, exchange->pitch * exchange->height);
	}
}

int main(int argc, char **argv)
{
	(void)argc;
	vbe = (struct vbe *)argv[1];
	printf("VBE: %dx%d\n", vbe->width, vbe->height);

	windows = list_new();
	root = new_window(0, 0, vbe->width, vbe->height);
	exchange = new_window(0, 0, vbe->width, vbe->height);
	cursor = new_window(0, 0, 16, 32);
	direct = malloc(sizeof(*direct));
	memcpy(direct, root, sizeof(*direct));
	direct->fb = vbe->fb;
	list_add(windows, root);

	const u32 background[3] = { 0x28, 0x2c, 0x34 };
	gui_fill(root, background);
	const u32 border[3] = { 0xab, 0xb2, 0xbf };
	gui_border(root, border, 2);
	gui_fill(cursor, border);
	// TODO: Fix wallpaper
	/* gui_load_wallpaper(root, "/wall.bmp"); */
	redraw_all();

	event_register(EVENT_KEYBOARD);
	event_register(EVENT_MOUSE);

	struct message *msg;
	while (1) {
		if (!(msg = msg_receive())) {
			yield();
			continue;
		}

		switch (msg->type) {
		case MSG_NEW_WINDOW:
			printf("New window for pid %d\n", msg->src);
			struct window *win =
				new_window(vbe->width / 2 - 100, vbe->height / 2 - 100, 500, 300);
			msg_send(msg->src, MSG_NEW_WINDOW, win);
			list_add(windows, win);
			focused = win;
			redraw_all();
			break;
		case MSG_REDRAW:
			redraw_all();
			break;
		case EVENT_KEYBOARD: {
			struct event_keyboard *event = msg->data;
			if (event->magic != KEYBOARD_MAGIC)
				break;
			/* printf("Keypress %d %s!\n", event->scancode, */
			/*        event->press ? "pressed" : "released"); */
			/* if (event->press) { */
			/* 	focused->x += 50; */
			/* 	redraw_all(); */
			/* } */
			break;
		}
		case EVENT_MOUSE: {
			struct event_mouse *event = msg->data;
			if (event->magic != MOUSE_MAGIC)
				break;
			mouse_x += event->diff_x;
			mouse_y -= event->diff_y;

			if (mouse_x < 0)
				mouse_x = 0;
			else if ((int)(mouse_x + cursor->width) > vbe->width - 1)
				mouse_x = vbe->width - cursor->width - 1;

			if (mouse_y < 0)
				mouse_y = 0;
			else if ((int)(mouse_y + cursor->height) > vbe->height - 1)
				mouse_y = vbe->height - cursor->height - 1;

			gui_copy(direct, exchange, cursor->x, cursor->y, cursor->width,
				 cursor->height);
			cursor->x = mouse_x;
			cursor->y = mouse_y;
			gui_win_on_win(direct, cursor, cursor->x, cursor->y);

			if (event->but1 && mouse_x + (int)focused->width < vbe->width - 1 &&
			    mouse_y + (int)focused->height < vbe->height - 1) {
				focused->x = mouse_x;
				focused->y = mouse_y;
				redraw_all(); // TODO: Function to redraw one window
			}
			break;
		}
		default:
			printf("Unknown WM request %d from pid %d\n", msg->type, msg->src);
		}
	};

	return 0;
}

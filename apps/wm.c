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

#define MOUSE_SKIP 5 // => Every move % n != 0 gets skipped

static struct vbe *vbe;
static struct window *direct; // Direct video memory window
static struct window *root; // Root window (wallpaper etc.)
static struct window *exchange; // Exchange buffer
static struct window *focused; // The focused window
static struct window *cursor; // Cursor bitmap window
static struct list *windows; // List of all windows

static int mouse_x = 0;
static int mouse_y = 0;

static struct window *new_window(int x, int y, u16 width, u16 height, int flags)
{
	struct window *win = malloc(sizeof(*win));
	win->x = x;
	win->y = y;
	win->width = width;
	win->height = height;
	win->bpp = vbe->bpp;
	win->pitch = win->width * (win->bpp >> 3);
	win->fb = malloc(height * win->pitch);
	win->flags = flags;
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

	gui_init("/font/spleen-16x32.psfu");

	windows = list_new();
	root = new_window(0, 0, vbe->width, vbe->height, WF_NO_FOCUS | WF_NO_DRAG | WF_NO_RESIZE);
	exchange =
		new_window(0, 0, vbe->width, vbe->height, WF_NO_FOCUS | WF_NO_DRAG | WF_NO_RESIZE);
	cursor = new_window(0, 0, 32, 32, WF_NO_FOCUS | WF_NO_RESIZE);
	direct = malloc(sizeof(*direct));
	memcpy(direct, root, sizeof(*direct));
	direct->fb = vbe->fb;
	list_add(windows, root);

	gui_write(direct, 0, 0, FG_COLOR, "Welcome to Melvix!");
	gui_write(direct, 0, 32, FG_COLOR, "Loading resources...");

	gui_fill(root, BG_COLOR);
	gui_border(root, FG_COLOR, 2);
	gui_load_image(cursor, "/res/cursor.bmp", 0, 0);
	gui_load_wallpaper(root, "/res/wall.bmp");
	redraw_all();

	event_register(EVENT_MOUSE);

	struct message *msg;
	int mouse_skip = 0;
	while (1) {
		if (!(msg = msg_receive())) {
			yield();
			continue;
		}

		switch (msg->type) {
		case MSG_NEW_WINDOW:
			printf("New window for pid %d\n", msg->src);
			struct window *win = new_window(vbe->width / 2 - 500, vbe->height / 2 - 400,
							1000, 800, (int)msg->data);
			msg_send(msg->src, MSG_NEW_WINDOW, win);
			list_add(windows, win);
			focused = win;
			redraw_all();
			break;
		case MSG_REDRAW:
			redraw_all();
			break;
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

			if (event->but1 && !(focused->flags & WF_NO_DRAG) &&
			    mouse_skip % MOUSE_SKIP == 0) {
				mouse_skip = 0;
				focused->x = mouse_x;
				focused->y = mouse_y;
				redraw_all(); // TODO: Function to redraw one window
			}
			gui_win_on_win(direct, cursor, cursor->x, cursor->y);
			mouse_skip++;
			break;
		}
		default:
			break;
		}
	};

	return 0;
}

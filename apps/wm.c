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

static struct vbe vbe;
static struct window direct; // Direct video memory window
static struct window root; // Root window (wallpaper etc.)
static struct window exchange; // Exchange buffer
static struct window cursor; // Cursor bitmap window
static struct window *focused; // The focused window
static struct list *windows; // List of all windows

static int mouse_x = 0;
static int mouse_y = 0;

static struct window *new_window(struct window *win, int x, int y, u16 width, u16 height, int flags)
{
	win->x = x;
	win->y = y;
	win->width = width;
	win->height = height;
	win->bpp = vbe.bpp;
	win->pitch = win->width * (win->bpp >> 3);
	win->fb = malloc(height * win->pitch);
	win->flags = flags;
	return win;
}

static struct window *window_at(int x, int y)
{
	if (windows->head && windows->head->data) {
		struct node *iterator = windows->head;
		do {
			struct window *win = iterator->data;
			if (win != &root && x >= win->x && x <= win->x + (int)win->width &&
			    y >= win->y && y <= win->y + (int)win->height) {
				return win;
			}
		} while ((iterator = iterator->next) != NULL);
	}
	return NULL;
}

static void redraw_all()
{
	if (windows->head && windows->head->data) {
		struct node *iterator = windows->head;
		do {
			struct window *win = iterator->data;
			gui_win_on_win(&exchange, win, win->x, win->y);
		} while ((iterator = iterator->next) != NULL);
		memcpy(direct.fb, exchange.fb, exchange.pitch * exchange.height);
	}
}

// TODO: Clean this god-function
int main(int argc, char **argv)
{
	(void)argc;
	vbe = *(struct vbe *)argv[1];
	printf("VBE: %dx%d\n", vbe.width, vbe.height);

	gui_init("/font/spleen-16x32.psfu");

	windows = list_new();
	new_window(&root, 0, 0, vbe.width, vbe.height, WF_NO_FOCUS | WF_NO_DRAG | WF_NO_RESIZE);
	new_window(&exchange, 0, 0, vbe.width, vbe.height, WF_NO_FOCUS | WF_NO_DRAG | WF_NO_RESIZE);
	new_window(&cursor, 0, 0, 32, 32, WF_NO_FOCUS | WF_NO_RESIZE);
	memcpy(&direct, &root, sizeof(direct));
	direct.fb = vbe.fb;
	list_add(windows, &root);

	gui_fill(&direct, COLOR_BG);
	gui_write(&direct, 0, 0, COLOR_FG, "Welcome to Melvix!");
	gui_write(&direct, 0, 32, COLOR_FG, "Loading resources...");

	gui_fill(&root, COLOR_BG);
	gui_border(&root, COLOR_FG, 2);
	gui_load_image(&cursor, "/res/cursor.bmp", 0, 0);
	gui_load_wallpaper(&root, "/res/wall.bmp");
	redraw_all();

	event_register(EVENT_MOUSE);

	struct message *msg;
	int mouse_skip = 0;
	int but1_pressed = 0;
	while (1) {
		if (!(msg = msg_receive())) {
			yield();
			continue;
		}

		switch (msg->type) {
		case MSG_NEW_WINDOW:
			printf("New window for pid %d\n", msg->src);
			struct window *win = msg->data;
			int width = win->width ? win->width : 1000;
			int height = win->height ? win->height : 800;
			int x = win->x ? win->x : vbe.width / 2 - (width / 2);
			int y = win->y ? win->y : vbe.height / 2 - (height / 2);
			new_window(win, x, y, width, height, win->flags);
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
			else if ((int)(mouse_x + cursor.width) > vbe.width - 1)
				mouse_x = vbe.width - cursor.width - 1;

			if (mouse_y < 0)
				mouse_y = 0;
			else if ((int)(mouse_y + cursor.height) > vbe.height - 1)
				mouse_y = vbe.height - cursor.height - 1;

			gui_copy(&direct, &exchange, cursor.x, cursor.y, cursor.width,
				 cursor.height);
			cursor.x = mouse_x;
			cursor.y = mouse_y;
			if (!but1_pressed)
				focused = window_at(cursor.x, cursor.y);

			if (event->but1) {
				but1_pressed = 1;
				if (focused && !(focused->flags & WF_NO_DRAG) &&
				    mouse_skip % MOUSE_SKIP == 0) {
					mouse_skip = 0;
					focused->x = cursor.x;
					focused->y = cursor.y;
					redraw_all(); // TODO: Function to redraw one window
				}
			} else {
				but1_pressed = 0;
			}
			gui_win_on_win(&direct, &cursor, cursor.x, cursor.y);
			mouse_skip++;
			break;
		}
		default:
			break;
		}
	};

	return 0;
}

// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <cpu.h>
#include <def.h>
#include <gui.h>
#include <input.h>
#include <keymap.h>
#include <list.h>
#include <mem.h>
#include <print.h>
#include <random.h>
#include <sys.h>
#include <vesa.h>

static int MOUSE_SKIP = 0; // => Every move % n != 0 gets skipped
static int window_count;

static struct vbe vbe;
static struct window direct; // Direct video memory window
static struct window root; // Root window (wallpaper etc.)
static struct window exchange; // Exchange buffer
static struct window cursor; // Cursor bitmap window
static struct window *focused; // The focused window
static struct list *windows; // List of all windows

static struct keymap *keymap;

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
	memset(win->fb, 0, height * win->pitch);
	win->flags = flags;
	window_count++;
	if (window_count % 2 == 1)
		MOUSE_SKIP++;
	return win;
}

static struct window *window_at(int x, int y)
{
	if (!windows->head || !windows->head->data)
		return NULL;

	struct window *ret = NULL;
	struct node *iterator = windows->head;
	while (iterator != NULL) {
		struct window *win = iterator->data;
		if (win != &root && x >= win->x && x <= win->x + (int)win->width && y >= win->y &&
		    y <= win->y + (int)win->height)
			ret = win;
		iterator = iterator->next;
	}
	return ret;
}

static void redraw_all()
{
	if (!windows->head || !windows->head->data)
		return;

	struct node *iterator = windows->head;
	while (iterator != NULL) {
		struct window *win = iterator->data;
		if (win != focused)
			gui_win_on_win(&exchange, win, win->x, win->y);
		iterator = iterator->next;
	}

	if (focused)
		gui_win_on_win(&exchange, focused, focused->x, focused->y);

	memcpy(direct.fb, exchange.fb, exchange.pitch * exchange.height);
}

static int mouse_skip = 0;
static int mouse_pressed[3] = { 0 };
static void handle_mouse(struct event_mouse *event)
{
	if (event->magic != MOUSE_MAGIC)
		return;

	// Cursor movement
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

	// Restore cursor buffer backup
	gui_copy(&direct, &exchange, cursor.x, cursor.y, cursor.width, cursor.height);

	// Window focus
	if (!mouse_pressed[0] && !mouse_pressed[1])
		focused = window_at(mouse_x, mouse_y);

	// Window position
	if (event->but1 && !mouse_pressed[1]) {
		mouse_pressed[0] = 1;
		if (focused && !(focused->flags & WF_NO_DRAG)) {
			focused->x = mouse_x;
			focused->y = mouse_y;
			if (mouse_skip % MOUSE_SKIP == 0) {
				mouse_skip = 0;
				redraw_all(); // TODO: Function to redraw one window
			}
		}
	} else if (mouse_pressed[0]) {
		mouse_pressed[0] = 0;
		redraw_all();
	}

	// Window size
	if (event->but2 && !mouse_pressed[0]) {
		if (focused && !mouse_pressed[1]) {
			mouse_x = focused->x + focused->width;
			mouse_y = focused->y + focused->height;
		} else if (focused && !(focused->flags & WF_NO_RESIZE) &&
			   mouse_skip % MOUSE_SKIP == 0) {
			mouse_skip = 0;
			if (mouse_x - focused->x > 0)
				focused->width = mouse_x - focused->x;
			if (mouse_y - focused->y > 0)
				focused->height = mouse_y - focused->y;
			redraw_all(); // TODO: Function to redraw one window
		}
		mouse_pressed[1] = 1;
	} else if (mouse_pressed[1]) {
		mouse_pressed[1] = 0;
		redraw_all();
	}

	cursor.x = mouse_x;
	cursor.y = mouse_y;
	gui_win_on_win(&direct, &cursor, cursor.x, cursor.y);
	mouse_skip++;
}

// TODO: Clean this god-function
int main(int argc, char **argv)
{
	(void)argc;
	vbe = *(struct vbe *)argv[1];
	printf("VBE: %dx%d\n", vbe.width, vbe.height);

	keymap = keymap_parse("/res/keymaps/en.keymap");
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
		case EVENT_MOUSE:
			handle_mouse(msg->data);
			break;
		default:
			break;
		}
	};

	return 0;
}

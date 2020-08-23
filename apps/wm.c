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
struct list *windows;

void onkey(u32 scancode)
{
	printf("WM KEY EVENT %d\n", scancode);
	if (KEY_ALPHANUMERIC(scancode)) {
		printf("ALPHANUMERIC!\n");
	}
	event_resolve();
}

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
	direct = malloc(sizeof(*direct));
	memcpy(direct, root, sizeof(*direct));
	direct->fb = vbe->fb;

	const u32 background[3] = { 0x10, 0x10, 0x10 };
	gui_fill(root, background);
	gui_load_wallpaper(root, "/wall.bmp");
	list_add(windows, root);

	// TODO: Remove async events completely
	/* event_map(EVENT_KEYBOARD, onkey); */

	u32 last_time = 0;
	struct message *msg;
	while (1) {
		u32 current_time = time();
		if (current_time - last_time > 1000 / 60) { // 60Hz
			struct window *win;
			if (windows->head && (win = windows->head->data))
				gui_win_on_win(win, direct, 0, 0);
		}
		last_time = current_time;

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
		default:
			printf("Unknown WM request %d from pid %d", msg->type, msg->src);
		}
	};

	return 0;
}

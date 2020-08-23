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
	win->vbe = vbe;
	win->fb = malloc(width * height * (vbe->bpp >> 3));
	return win;
}

int main(int argc, char **argv)
{
	(void)argc;
	vbe = (struct vbe *)argv[1];
	windows = list_new();

	printf("VBE: %dx%d\n", vbe->width, vbe->height);

	const u32 color[3] = { 0, 0, 0 };
	vesa_fill(vbe, color);
	gui_init("/font/spleen-16x32.psfu");
	gui_load_wallpaper(vbe, "/wall.bmp");

	event_map(EVENT_KEYBOARD, onkey);

	u32 last_time = 0;
	struct message *msg;
	while (1) { // TODO: Remove continuous polling?
		/* u32 current_time = time(); */
		/* if (current_time - last_time > 1000 / 60) { // 60Hz */
		/* 	struct window *win = windows->head->data; */
		/* 	memcpy(vbe->fb, windows->head->data, */
		/* 	       win->width * win->height * (vbe->bpp >> 3)); */
		/* } */
		/* last_time = current_time; */

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

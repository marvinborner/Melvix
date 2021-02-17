// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <def.h>
#include <gfx.h>
#include <input.h>
#include <keymap.h>
#include <list.h>
#include <random.h>
#include <vesa.h>

struct client {
	u32 pid;
};

struct window {
	u32 id;
	const char *name;
	struct context ctx;
	struct client client;
	u32 flags;
	vec3 pos;
};

static struct vbe screen = { 0 };
static struct list *windows = NULL;
static struct window *root = NULL;
static struct window *cursor = NULL;
static struct keymap *keymap = NULL;
static struct {
	u8 shift : 1;
	u8 alt : 1;
	u8 ctrl : 1;
} special_keys = { 0 };
static struct {
	vec2 pos;
	u8 left : 1;
	u8 mid : 1;
	u8 right : 1;
} mouse = { 0 };

static struct window *window_create(struct client client, const char *name, struct vec3 pos,
				    struct vec2 size, u32 flags)
{
	struct window *win = malloc(sizeof(*win));
	win->id = rand();
	win->name = name;
	win->ctx.size = size;
	win->ctx.bpp = screen.bpp;
	win->ctx.pitch = size.x * (win->ctx.bpp >> 3);
	if ((flags & WF_NO_FB) == 0)
		win->ctx.fb = malloc(size.y * win->ctx.pitch);
	win->client = client;
	win->flags = flags;
	win->pos = pos;
	list_add(windows, win);
	return win;
}

static struct window *window_find(u32 id)
{
	struct node *iterator = windows->head;
	while (iterator) {
		struct window *win = iterator->data;
		if (win->id == id)
			return win;
		iterator = iterator->next;
	}
	return NULL;
}

static void window_destroy(struct window *win)
{
	free(win->ctx.fb);
	free(win);
}

static void handle_event_keyboard(struct event_keyboard *event)
{
	if (event->magic != KEYBOARD_MAGIC) {
		log("Keyboard magic doesn't match!\n");
		return;
	}

	if (event->scancode == KEY_LEFTSHIFT || event->scancode == KEY_RIGHTSHIFT)
		special_keys.shift ^= 1;
	else if (event->scancode == KEY_LEFTALT || event->scancode == KEY_RIGHTALT)
		special_keys.alt ^= 1;
	else if (event->scancode == KEY_LEFTCTRL || event->scancode == KEY_RIGHTCTRL)
		special_keys.ctrl ^= 1;

	char ch;
	if (special_keys.shift)
		ch = keymap->shift_map[event->scancode];
	else if (special_keys.alt)
		ch = keymap->alt_map[event->scancode];
	else
		ch = keymap->map[event->scancode];

	(void)ch;
}

static void handle_event_mouse(struct event_mouse *event)
{
	if (event->magic != MOUSE_MAGIC) {
		log("Mouse magic doesn't match!\n");
		return;
	}

	mouse.pos.x += event->diff_x;
	mouse.pos.y -= event->diff_y;

	// Fix x overflow
	if ((signed)mouse.pos.x < 0)
		mouse.pos.x = 0;
	else if (mouse.pos.x + cursor->ctx.size.x > (unsigned)screen.width - 1)
		mouse.pos.x = screen.width - cursor->ctx.size.x - 1;

	// Fix y overflow
	if ((signed)mouse.pos.y < 0)
		mouse.pos.y = 0;
	else if (mouse.pos.y + cursor->ctx.size.y > (unsigned)screen.height - 1)
		mouse.pos.y = screen.height - cursor->ctx.size.y - 1;

	//log("%d %d\n", mouse.pos.x, mouse.pos.y);
	cursor->pos = vec2to3(mouse.pos, U32_MAX);

	gfx_fill(&cursor->ctx, COLOR_RED);
	if (event->but1) {
		/* TODO: Fix gfx_copy! */
		/* gfx_copy(&root->ctx, &cursor->ctx, vec3to2(cursor->pos), cursor->ctx.size); */
		gfx_draw_rectangle(&root->ctx, vec3to2(cursor->pos),
				   vec2_add(cursor->pos, cursor->ctx.size), COLOR_RED);
	}
}

int main(int argc, char **argv)
{
	(void)argc;
	int pid = getpid();
	screen = *(struct vbe *)argv[1];
	log("WM loaded: %dx%d\n", screen.width, screen.height);

	windows = list_new();
	keymap = keymap_parse("/res/keymaps/en.keymap");

	root = window_create((struct client){ pid }, "root", vec3(0, 0, 0),
			     vec2(screen.width, screen.height), WF_NO_FB);
	root->ctx.fb = screen.fb;
	root->flags ^= WF_NO_FB;
	cursor = window_create((struct client){ pid }, "cursor", vec3(0, 0, 0), vec2(20, 20),
			       WF_NO_DRAG | WF_NO_FOCUS | WF_NO_RESIZE);

	gfx_fill(&root->ctx, COLOR_WHITE);

	struct message msg = { 0 };
	struct event_keyboard event_keyboard = { 0 };
	struct event_mouse event_mouse = { 0 };
	const char *listeners[] = { "/dev/kbd", "/dev/mouse", "/proc/self/msg" };
	while (1) {
		int poll_ret = 0;
		if ((poll_ret = poll(listeners)) >= 0) {
			if (poll_ret == 0) {
				if (read(listeners[poll_ret], &event_keyboard, 0,
					 sizeof(event_keyboard)) > 0)
					handle_event_keyboard(&event_keyboard);
				continue;
			} else if (poll_ret == 1) {
				if (read(listeners[poll_ret], &event_mouse, 0,
					 sizeof(event_mouse)) > 0)
					handle_event_mouse(&event_mouse);
				continue;
			} else if (poll_ret == 2) {
				if (read(listeners[poll_ret], &msg, 0, sizeof(msg)) <= 0)
					continue;
			}
		} else {
			err(1, "POLL ERROR!\n");
		}

		if (msg.magic != MSG_MAGIC) {
			log("Message magic doesn't match!\n");
			continue;
		}

		log("not implemented!\n");
	};

	// TODO: Execute?
	free(keymap);
	list_destroy(windows);

	return 0;
}

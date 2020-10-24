// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <cpu.h>
#include <def.h>
#include <gfx.h>
#include <input.h>
#include <keymap.h>
#include <list.h>
#include <mem.h>
#include <print.h>
#include <random.h>
#include <sys.h>
#include <vesa.h>

static int MOUSE_SKIP = 0; // => Every move % n != 0 gets skipped
static int context_count;

static struct vbe vbe;
static struct context direct; // Direct video memory context
static struct context root; // Root context (wallpaper etc.)
static struct context exchange; // Exchange buffer
static struct context cursor; // Cursor bitmap context
static struct context *focused; // The focused context
static struct list *contexts; // List of all contexts

static struct keymap *keymap;

static int mouse_x = 0;
static int mouse_y = 0;

static struct context *new_context(struct context *ctx, u32 pid, int x, int y, u16 width,
				   u16 height, int flags)
{
	ctx->pid = pid;
	ctx->x = x;
	ctx->y = y;
	ctx->width = width;
	ctx->height = height;
	ctx->bpp = vbe.bpp;
	ctx->pitch = ctx->width * (ctx->bpp >> 3);
	ctx->fb = malloc(height * ctx->pitch);
	memset(ctx->fb, 0, height * ctx->pitch);
	ctx->flags = flags;
	context_count++;
	if (context_count % 2 == 1)
		MOUSE_SKIP++;
	return ctx;
}

static struct context *context_at(int x, int y)
{
	if (!contexts->head || !contexts->head->data)
		return NULL;

	struct context *ret = NULL;
	struct node *iterator = contexts->head;
	while (iterator != NULL) {
		struct context *ctx = iterator->data;
		if (ctx != &root && x >= ctx->x && x <= ctx->x + (int)ctx->width && y >= ctx->y &&
		    y <= ctx->y + (int)ctx->height)
			ret = ctx;
		iterator = iterator->next;
	}
	return ret;
}

static void redraw_all()
{
	if (!contexts->head || !contexts->head->data)
		return;

	struct node *iterator = contexts->head;
	while (iterator != NULL) {
		struct context *ctx = iterator->data;
		if (ctx != focused)
			gfx_ctx_on_ctx(&exchange, ctx, ctx->x, ctx->y);
		iterator = iterator->next;
	}

	if (focused)
		gfx_ctx_on_ctx(&exchange, focused, focused->x, focused->y);

	memcpy(direct.fb, exchange.fb, exchange.pitch * exchange.height);
}

// TODO: Send relative mouse position event to focused context
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
	gfx_copy(&direct, &exchange, cursor.x, cursor.y, cursor.width, cursor.height);

	// Context focus
	if (!mouse_pressed[0] && !mouse_pressed[1])
		focused = context_at(mouse_x, mouse_y);

	// Context position
	if (event->but1 && !mouse_pressed[1]) {
		mouse_pressed[0] = 1;
		if (focused && !(focused->flags & WF_NO_DRAG)) {
			focused->x = mouse_x;
			focused->y = mouse_y;
			if (mouse_skip % MOUSE_SKIP == 0) {
				mouse_skip = 0;
				redraw_all(); // TODO: Function to redraw one context
			}
		}
	} else if (mouse_pressed[0]) {
		mouse_pressed[0] = 0;
		redraw_all();
	}

	// Context size
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
			redraw_all(); // TODO: Function to redraw one context
		}
		mouse_pressed[1] = 1;
	} else if (mouse_pressed[1]) {
		mouse_pressed[1] = 0;
		redraw_all();
	}

	cursor.x = mouse_x;
	cursor.y = mouse_y;
	gfx_ctx_on_ctx(&direct, &cursor, cursor.x, cursor.y);
	mouse_skip++;
}

#define SHIFT_PRESSED 1 << 0
#define ALT_PRESSED 1 << 1
#define CTRL_PRESSED 1 << 2
static u32 special_keys_pressed;
static void handle_keyboard(struct event_keyboard *event)
{
	if (event->magic != KEYBOARD_MAGIC || !focused)
		return;

	struct msg_keyboard *msg = malloc(sizeof(*msg));
	if (event->scancode == KEY_LEFTSHIFT || event->scancode == KEY_RIGHTSHIFT)
		special_keys_pressed ^= SHIFT_PRESSED;
	else if (event->scancode == KEY_LEFTALT || event->scancode == KEY_RIGHTALT)
		special_keys_pressed ^= ALT_PRESSED;
	else if (event->scancode == KEY_LEFTCTRL || event->scancode == KEY_RIGHTCTRL)
		special_keys_pressed ^= CTRL_PRESSED;

	if (special_keys_pressed & SHIFT_PRESSED)
		msg->ch = keymap->shift_map[event->scancode];
	else
		msg->ch = keymap->map[event->scancode];

	msg->press = event->press;
	msg->scancode = event->scancode;
	msg_send(focused->pid, WM_KEYBOARD, msg);
}

// TODO: Clean this god-function
int main(int argc, char **argv)
{
	(void)argc;
	int pid = getpid();
	vbe = *(struct vbe *)argv[1];
	printf("VBE: %dx%d\n", vbe.width, vbe.height);

	keymap = keymap_parse("/res/keymaps/en.keymap");
	gfx_init("/font/spleen-16x32.psfu");

	contexts = list_new();
	new_context(&root, pid, 0, 0, vbe.width, vbe.height,
		    WF_NO_FOCUS | WF_NO_DRAG | WF_NO_RESIZE);
	new_context(&exchange, pid, 0, 0, vbe.width, vbe.height,
		    WF_NO_FOCUS | WF_NO_DRAG | WF_NO_RESIZE);
	new_context(&cursor, pid, 0, 0, 32, 32, WF_NO_FOCUS | WF_NO_RESIZE);
	memcpy(&direct, &root, sizeof(direct));
	direct.fb = vbe.fb;
	list_add(contexts, &root);

	gfx_fill(&direct, COLOR_BG);
	gfx_write(&direct, 0, 0, COLOR_FG, "Welcome to Melvix!");
	gfx_write(&direct, 0, 32, COLOR_FG, "Loading resources...");

	gfx_fill(&root, COLOR_BG);
	gfx_border(&root, COLOR_FG, 2);
	gfx_load_image(&cursor, "/res/cursor.bmp", 0, 0);
	gfx_load_wallpaper(&root, "/res/wall.bmp");
	redraw_all();

	event_register(EVENT_MOUSE);
	event_register(EVENT_KEYBOARD);

	struct message *msg;
	while (1) {
		if (!(msg = msg_receive())) {
			yield();
			continue;
		}

		switch (msg->type) {
		case WM_NEW_CONTEXT:
			printf("New context for pid %d\n", msg->src);
			struct context *ctx = msg->data;
			int width = ctx->width ? ctx->width : 1000;
			int height = ctx->height ? ctx->height : 800;
			int x = ctx->x ? ctx->x : vbe.width / 2 - (width / 2);
			int y = ctx->y ? ctx->y : vbe.height / 2 - (height / 2);
			ctx->pid = msg->src;
			new_context(ctx, msg->src, x, y, width, height, ctx->flags);
			list_add(contexts, ctx);
			focused = ctx;
			redraw_all();
			msg_send(msg->src, WM_NEW_CONTEXT, ctx);
			break;
		case WM_REDRAW:
			redraw_all();
			break;
		case EVENT_MOUSE:
			handle_mouse(msg->data);
			break;
		case EVENT_KEYBOARD:
			handle_keyboard(msg->data);
			break;
		default:
			break;
		}
	};

	return 0;
}

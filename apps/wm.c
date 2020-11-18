// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <cpu.h>
#include <def.h>
#include <gfx.h>
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
	if (!(flags & WF_RELATIVE)) {
		context_count++;
		if (context_count % 2 == 1)
			MOUSE_SKIP++;
	}
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
		if (ctx != &root && !(ctx->flags & WF_RELATIVE) && x >= ctx->x &&
		    x <= ctx->x + (int)ctx->width && y >= ctx->y && y <= ctx->y + (int)ctx->height)
			ret = ctx;
		iterator = iterator->next;
	}
	return ret;
}

// This only works if window hasn't moved - TODO!
static void redraw_focused()
{
	gfx_ctx_on_ctx(&exchange, focused, focused->x, focused->y);
	memcpy(direct.fb, exchange.fb, exchange.pitch * exchange.height);
}

// TODO: Add dirty bitmap redraw (and clipping?): https://github.com/JMarlin/wsbe
static void redraw_all()
{
	if (!contexts->head || !contexts->head->data)
		return;

	struct node *iterator = contexts->head;
	while (iterator != NULL) {
		struct context *ctx = iterator->data;
		if (ctx != focused && !(ctx->flags & WF_RELATIVE))
			gfx_ctx_on_ctx(&exchange, ctx, ctx->x, ctx->y);
		iterator = iterator->next;
	}

	if (focused)
		gfx_ctx_on_ctx(&exchange, focused, focused->x, focused->y);

	memcpy(direct.fb, exchange.fb, exchange.pitch * exchange.height);
}

#define SHIFT_PRESSED 1 << 0
#define ALT_PRESSED 1 << 1
#define CTRL_PRESSED 1 << 2
static u32 special_keys_pressed;
static void handle_keyboard(struct event_keyboard *event)
{
	if (event->magic != KEYBOARD_MAGIC)
		return;

	if (event->scancode == KEY_LEFTSHIFT || event->scancode == KEY_RIGHTSHIFT)
		special_keys_pressed ^= SHIFT_PRESSED;
	else if (event->scancode == KEY_LEFTALT || event->scancode == KEY_RIGHTALT)
		special_keys_pressed ^= ALT_PRESSED;
	else if (event->scancode == KEY_LEFTCTRL || event->scancode == KEY_RIGHTCTRL)
		special_keys_pressed ^= CTRL_PRESSED;

	if (!focused)
		return;

	struct gui_event_keyboard *msg = malloc(sizeof(*msg));

	if (special_keys_pressed & SHIFT_PRESSED)
		msg->ch = keymap->shift_map[event->scancode];
	else if (special_keys_pressed & ALT_PRESSED)
		msg->ch = keymap->alt_map[event->scancode];
	else
		msg->ch = keymap->map[event->scancode];

	msg->press = event->press;
	msg->scancode = event->scancode;
	msg_send(focused->pid, GUI_KEYBOARD, msg);
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
	gfx_copy(&direct, &exchange, cursor.x, cursor.y, cursor.width, cursor.height);

	int mod_pressed = special_keys_pressed & ALT_PRESSED;

	// Context focus
	if (!mouse_pressed[0] && !mouse_pressed[1])
		focused = context_at(mouse_x, mouse_y);

	// Context position
	if (mod_pressed && event->but1 && !mouse_pressed[1]) {
		mouse_pressed[0] = 1;
		if (focused && !(focused->flags & WF_NO_DRAG)) {
			focused->x = mouse_x;
			focused->y = mouse_y;
			if (mouse_skip % MOUSE_SKIP == 0) {
				mouse_skip = 0;
				redraw_all(); // TODO: Function to redraw one context
			}
		}
	} else if (mod_pressed && mouse_pressed[0]) {
		mouse_pressed[0] = 0;
		redraw_all();
	}

	// Context size
	if (mod_pressed && event->but2 && !mouse_pressed[0]) {
		if (focused && !mouse_pressed[1]) {
			mouse_x = focused->x + focused->width;
			mouse_y = focused->y + focused->height;
		} else if (focused && !(focused->flags & WF_NO_RESIZE) &&
			   mouse_skip % MOUSE_SKIP == 0) {
			mouse_skip = 0;
			if (mouse_x - focused->x > 0)
				focused->width = mouse_x - focused->x;
			if (mouse_y - focused->y > 0) {
				focused->height = mouse_y - focused->y;
				focused->pitch = focused->height * (focused->bpp >> 3);
			}
			redraw_all(); // TODO: Function to redraw one context
		}
		mouse_pressed[1] = 1;
	} else if (mod_pressed && mouse_pressed[1]) {
		mouse_pressed[1] = 0;
		redraw_all();
	}

	cursor.x = mouse_x;
	cursor.y = mouse_y;
	gfx_ctx_on_ctx(&direct, &cursor, cursor.x, cursor.y);
	mouse_skip++;

	if (!focused)
		return;
	struct gui_event_mouse *msg = malloc(sizeof(*msg));
	msg->x = mouse_x - focused->x;
	msg->y = mouse_y - focused->y;
	msg->but1 = event->but1;
	msg->but2 = event->but2;
	msg->but3 = event->but3;
	msg_send(focused->pid, GUI_MOUSE, msg);
}

// TODO: Clean this god-function
int main(int argc, char **argv)
{
	(void)argc;
	int pid = getpid();
	vbe = *(struct vbe *)argv[1];
	/* printf("VBE: %dx%d\n", vbe.width, vbe.height); */

	keymap = keymap_parse("/res/keymaps/en.keymap");

	contexts = list_new();
	new_context(&root, pid, 0, 0, vbe.width, vbe.height,
		    WF_NO_FOCUS | WF_NO_DRAG | WF_NO_RESIZE);
	new_context(&exchange, pid, 0, 0, vbe.width, vbe.height,
		    WF_NO_FOCUS | WF_NO_DRAG | WF_NO_RESIZE);
	new_context(&cursor, pid, 0, 0, 32, 32, WF_NO_FOCUS | WF_NO_RESIZE);
	memcpy(&direct, &root, sizeof(direct));
	direct.fb = vbe.fb;
	list_add(contexts, &root);

	/* gfx_write(&direct, 0, 0, FONT_32, COLOR_FG, "Welcome to Melvix!"); */
	/* gfx_write(&direct, 0, 32, FONT_32, COLOR_FG, "Loading resources..."); */
	gfx_fill(&root, COLOR_FG);
	/* gfx_load_wallpaper(&root, "/res/wall.bmp"); */
	gfx_load_image(&cursor, "/res/cursor.bmp", 0, 0);
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
		case GFX_NEW_CONTEXT: {
			struct context *ctx = msg->data;
			int width = ctx->width;
			int height = ctx->height;
			int x = ctx->x;
			int y = ctx->y;
			ctx->pid = msg->src;
			new_context(ctx, msg->src, x, y, width, height, ctx->flags);
			list_add(contexts, ctx);
			if (!(ctx->flags & WF_RELATIVE))
				focused = ctx;
			redraw_all();
			msg_send(msg->src, GFX_NEW_CONTEXT, ctx);
			break;
		}
		case GFX_REDRAW:
			redraw_all();
			break;
		case GFX_REDRAW_FOCUSED:
			redraw_focused();
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

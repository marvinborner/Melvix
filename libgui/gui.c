// MIT License, Copyright (c) 2020 Marvin Borner
// Mostly GFX function wrappers

#include <def.h>
#include <gfx.h>
#include <gui.h>
#include <list.h>
#include <mem.h>
#include <print.h>
#include <str.h>
#include <sys.h>

#define MAX_WINDOWS 10

u32 window_count = 0;
static struct window windows[MAX_WINDOWS] = { 0 };

static struct window *new_window(const char *title, int x, int y, u32 width, u32 height, int flags)
{
	if (window_count + 1 >= MAX_WINDOWS)
		return NULL;

	struct window *win = &windows[window_count + 1];
	win->ctx = malloc(sizeof(*win->ctx));
	win->ctx->x = x > 0 ? x : 50;
	win->ctx->y = y > 0 ? y : 50;
	win->ctx->width = width > 0 ? width : 600;
	win->ctx->height = height > 0 ? height : 400;
	win->ctx->flags = flags;
	win->id = window_count + 1;
	win->title = title;
	win->childs = list_new();
	gfx_new_ctx(win->ctx);

	if (!win->ctx->fb)
		return NULL;

	window_count++;

	return win;
}

static void merge_elements(struct element *container)
{
	if (!container || !container->childs || !container->childs->head)
		return;

	struct node *iterator = container->childs->head;
	while (iterator != NULL) {
		struct element *elem = iterator->data;
		struct context *ctx = elem->ctx;
		merge_elements(elem);
		gfx_ctx_on_ctx(container->ctx, ctx, ctx->x, ctx->y);
		iterator = iterator->next;
	}
}

static struct element *element_at(struct element *container, int x, int y)
{
	if (!container || !container->childs || !container->childs->head)
		return NULL;

	struct element *ret = NULL;
	struct node *iterator = container->childs->head;
	while (iterator != NULL) {
		struct context *ctx = ((struct element *)iterator->data)->ctx;
		if (ctx != container->ctx && ctx->flags & WF_RELATIVE && x >= ctx->x &&
		    x <= ctx->x + (int)ctx->width && y >= ctx->y && y <= ctx->y + (int)ctx->height)
			ret = iterator->data;
		iterator = iterator->next;
	}

	return ret;
}

void gui_sync_button(struct element *elem)
{
	struct element_button *button = elem->data;
	gfx_fill(elem->ctx, button->color_bg);
	gfx_write(elem->ctx, 0, 0, button->font_type, button->color_fg, button->text);
}

struct element_button *gui_add_button(struct element *container, int x, int y,
				      enum font_type font_type, char *text, u32 color_bg,
				      u32 color_fg)
{
	if (!container || !container->childs)
		return NULL;

	gfx_resolve_font(font_type);

	struct element *button = malloc(sizeof(*button));
	button->type = GUI_TYPE_BUTTON;
	button->window_id = container->window_id;
	button->ctx = malloc(sizeof(*button->ctx));
	button->ctx->x = x;
	button->ctx->y = y;
	button->ctx->width = strlen(text) * gfx_font_width(font_type);
	button->ctx->height = gfx_font_height(font_type);
	button->ctx->flags = WF_RELATIVE;
	button->childs = list_new();
	button->data = malloc(sizeof(struct element_button));
	((struct element_button *)button->data)->text = text;
	((struct element_button *)button->data)->color_fg = color_fg;
	((struct element_button *)button->data)->color_bg = color_bg;
	((struct element_button *)button->data)->font_type = font_type;
	gfx_new_ctx(button->ctx);
	list_add(container->childs, button);
	gui_sync_button(button);
	merge_elements(container);

	return button->data;
}

void gui_event_loop(struct element *container)
{
	if (!container)
		return;

	struct message *msg;
	while (1) {
		if (!(msg = msg_receive())) {
			yield();
			continue;
		}

		switch (msg->type) {
		case GUI_MOUSE: {
			struct gui_event_mouse *event = msg->data;
			struct element *elem = element_at(container, event->x, event->y);
			if (!elem)
				continue;

			if (elem->type == GUI_TYPE_BUTTON) {
				struct element_button *button = elem->data;
				if (event->but1 && button->on_click)
					button->on_click();
			}
		}
		}
	}
}

struct element *gui_init(const char *title, u32 width, u32 height)
{
	if (window_count != 0)
		return NULL;

	struct window *win = new_window(title, 0, 0, width, height, WF_DEFAULT);
	if (!win)
		return NULL;

	gfx_fill(win->ctx, COLOR_BG);

	struct element *container = malloc(sizeof(*container));
	container->type = GUI_TYPE_CONTAINER;
	container->window_id = win->id;
	container->ctx = win->ctx;
	container->childs = list_new();
	container->data = NULL;
	list_add(win->childs, container);

	return container;
}

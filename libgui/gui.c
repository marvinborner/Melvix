// MIT License, Copyright (c) 2020 Marvin Borner
// Mostly GFX function wrappers

#include <def.h>
#include <gfx.h>
#include <gui.h>
#include <list.h>
#include <mem.h>

#define MAX_WINDOWS 10

u32 window_count = 0;
static struct window windows[MAX_WINDOWS] = { 0 };

struct window *new_window(const char *title, int x, int y, u32 width, u32 height, int flags)
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

void merge_elements(struct element *container)
{
	if (!container->childs || !container->childs->head)
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

struct element *gui_add_button(struct element *container, int x, int y, u32 width, u32 height,
			       const char *text, u32 color)
{
	if (!container || !container->childs)
		return NULL;

	struct element *button = malloc(sizeof(*button));
	button->type = GUI_TYPE_BUTTON;
	button->window_id = container->window_id;
	button->ctx = malloc(sizeof(*button->ctx));
	button->ctx->x = x;
	button->ctx->y = y;
	button->ctx->width = width;
	button->ctx->height = height;
	button->ctx->flags = WF_RELATIVE;
	button->childs = list_new();
	button->data = malloc(sizeof(struct element_button));
	((struct element_button *)button->data)->text = text;
	((struct element_button *)button->data)->color = color;
	gfx_new_ctx(button->ctx);
	gfx_fill(button->ctx, color);
	list_add(container->childs, button);
	merge_elements(container);

	return button;
}

struct element *gui_init(const char *title, u32 width, u32 height)
{
	if (window_count != 0)
		return NULL;

	struct window *win = new_window(title, 0, 0, width, height, WF_DEFAULT);
	if (!win)
		return NULL;

	gfx_fill(win->ctx, COLOR_BG);
	gfx_init("/font/spleen-12x24.psfu");

	struct element *container = malloc(sizeof(*container));
	container->type = GUI_TYPE_CONTAINER;
	container->window_id = win->id;
	container->ctx = win->ctx;
	container->childs = list_new();
	container->data = NULL;
	list_add(win->childs, container);

	return container;
}

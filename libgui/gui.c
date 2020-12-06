// MIT License, Copyright (c) 2020 Marvin Borner
// Mostly GFX function wrappers
// TODO: Reduce code duplication

#include <assert.h>
#include <def.h>
#include <gfx.h>
#include <gui.h>
#include <input.h>
#include <list.h>
#include <mem.h>
#include <print.h>
#include <str.h>
#include <sys.h>

// TODO: Use list (and add index-based access)
#define MAX_WINDOWS 10
#define BORDER 2
#define PERC(c, e) ((u32)((double)(c) * (double)(e) / 100.0))

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

static struct element *get_root(u32 window_id)
{
	struct list *childs = windows[window_id].childs;
	if (!childs || !childs->head || !childs->head->data)
		return NULL;
	return childs->head->data;
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

static void free_context(struct context *ctx)
{
	if (!ctx)
		return;

	free(ctx->fb);
	ctx->fb = NULL;
	free(ctx);
	ctx = NULL;
}

static void remove_childs(struct element *elem);
static void remove_element(struct element *elem)
{
	if (!elem)
		return;

	remove_childs(elem);
	free_context(elem->ctx);
	free(elem->data);
	elem->data = NULL;
	free(elem);
	elem = NULL;
}

static void remove_childs(struct element *elem)
{
	if (!elem || !elem->childs || !elem->childs->head)
		return;

	struct node *iterator = elem->childs->head;
	while (iterator != NULL) {
		struct element *child = iterator->data;
		remove_element(child);
		iterator = iterator->next;
	}

	list_destroy(elem->childs);
}

static void remove_window(struct window *win)
{
	if (!win || !win->childs || !win->childs->head)
		return;

	struct node *iterator = win->childs->head;
	while (iterator != NULL) {
		struct element *child = iterator->data;
		remove_element(child);
		iterator = iterator->next;
	}

	free_context(win->ctx);
	list_destroy(win->childs);
}

static void remove_all()
{
	for (int i = 0; i < MAX_WINDOWS; i++) {
		remove_window(&windows[i]);
	}
}

static struct element *element_at(struct element *container, int x, int y)
{
	if (!container || !container->childs || !container->childs->head)
		return NULL;

	struct node *iterator = container->childs->head;
	while (iterator != NULL) {
		struct context *ctx = ((struct element *)iterator->data)->ctx;

		int relative_x, relative_y;
		if (container->type == GUI_TYPE_ROOT) {
			relative_x = ctx->x;
			relative_y = ctx->y;
		} else {
			relative_x = ctx->x + container->ctx->x;
			relative_y = ctx->y + container->ctx->y;
		}

		if (ctx != container->ctx && ctx->flags & WF_RELATIVE && x >= relative_x &&
		    x <= relative_x + (int)ctx->width && y >= relative_y &&
		    y <= relative_y + (int)ctx->height) {
			struct element *recursive = NULL;
			if ((recursive = element_at(iterator->data, x, y)))
				return recursive;
			else
				return iterator->data;
		}

		iterator = iterator->next;
	}

	return NULL;
}

static int absolute_x_off(struct element *elem)
{
	if (!elem->parent)
		return 0;

	int x = 0;

	struct element *iterator = elem;
	while ((iterator = iterator->parent) && iterator->ctx)
		if (iterator->parent)
			x += iterator->ctx->x;

	return x;
}

static int absolute_y_off(struct element *elem)
{
	if (!elem->parent)
		return 0;

	int y = 0;

	struct element *iterator = elem;
	while ((iterator = iterator->parent) && iterator->ctx) {
		if (iterator->parent)
			y += iterator->ctx->y;
	}

	return y;
}

struct context *gui_get_context(int x, int y, u32 width, u32 height)
{
	struct context *ctx = malloc(sizeof(*ctx));
	ctx->pid = getpid();
	ctx->x = x;
	ctx->y = y;
	ctx->width = width;
	ctx->height = height;
	ctx->bpp = 32; // TODO: Dynamic bpp
	ctx->pitch = ctx->width * (ctx->bpp >> 3);
	ctx->fb = malloc(ctx->height * ctx->pitch);
	memset(ctx->fb, 0, ctx->height * ctx->pitch);
	ctx->flags = WF_RELATIVE;
	return ctx;
}

void gui_sync_button(struct element *elem)
{
	assert(elem->type == GUI_TYPE_BUTTON);
	struct element_button *button = elem->data;

	if (!elem->ctx) {
		elem->ctx =
			gui_get_context(button->x, button->y,
					strlen(button->text) * gfx_font_width(button->font_type),
					gfx_font_height(button->font_type));
	}

	gfx_fill(elem->ctx, button->color_bg);
	gfx_write(elem->ctx, 0, 0, button->font_type, button->color_fg, button->text);
}

void gui_sync_label(struct element *elem)
{
	assert(elem->type == GUI_TYPE_LABEL);
	struct element_label *label = elem->data;

	if (!elem->ctx) {
		elem->ctx = gui_get_context(label->x, label->y,
					    strlen(label->text) * gfx_font_width(label->font_type),
					    gfx_font_height(label->font_type));
	}

	gfx_fill(elem->ctx, label->color_bg);
	gfx_write(elem->ctx, 0, 0, label->font_type, label->color_fg, label->text);
}

void gui_sync_text_box(struct element *elem)
{
	assert(elem->type == GUI_TYPE_TEXT_BOX);
	struct element_text_box *text_box = elem->data;

	int abs_x = absolute_x_off(elem) + text_box->x;
	int abs_y = absolute_y_off(elem) + text_box->y;
	if (!elem->ctx ||
	    PERC(elem->parent->ctx->width, text_box->width) - abs_x != elem->ctx->width ||
	    PERC(elem->parent->ctx->height, text_box->height) - abs_y != elem->ctx->height) {
		free_context(elem->ctx);
		elem->ctx =
			gui_get_context(text_box->x, text_box->y,
					PERC(elem->parent->ctx->width, text_box->width) - abs_x,
					PERC(elem->parent->ctx->height, text_box->height) - abs_y);
	}

	gfx_fill(elem->ctx, text_box->color_bg);
	gfx_write(elem->ctx, 0, 0, text_box->font_type, text_box->color_fg, text_box->text);
}

void gui_sync_text_input(struct element *elem)
{
	assert(elem->type == GUI_TYPE_TEXT_INPUT);
	struct element_text_input *text_input = elem->data;

	int abs_x = absolute_x_off(elem) + text_input->x;
	if (!elem->ctx ||
	    PERC(elem->parent->ctx->width, text_input->width) - abs_x != elem->ctx->width ||
	    (u32)gfx_font_height(text_input->font_type) != elem->ctx->height) {
		free_context(elem->ctx);
		elem->ctx =
			gui_get_context(text_input->x, text_input->y,
					PERC(elem->parent->ctx->width, text_input->width) - abs_x,
					gfx_font_height(text_input->font_type));
	}

	gfx_fill(elem->ctx, text_input->color_bg);
	gfx_write(elem->ctx, 0, 0, text_input->font_type, text_input->color_fg, text_input->text);
}

void gui_sync_container(struct element *elem)
{
	assert(elem->type == GUI_TYPE_CONTAINER);
	struct element_container *container = elem->data;

	int abs_x = absolute_x_off(elem) + container->x;
	int abs_y = absolute_y_off(elem) + container->y;
	if (!elem->ctx ||
	    PERC(elem->parent->ctx->width, container->width) - abs_x != elem->ctx->width ||
	    PERC(elem->parent->ctx->height, container->height) - abs_y != elem->ctx->height) {
		free_context(elem->ctx);
		elem->ctx =
			gui_get_context(container->x, container->y,
					PERC(elem->parent->ctx->width, container->width) - abs_x,
					PERC(elem->parent->ctx->height, container->height) - abs_y);
	}

	gfx_fill(elem->ctx, container->color_bg);
	// TODO: Handle container flags
}

void gui_only_sync(struct element *elem)
{
	switch (elem->type) {
	case GUI_TYPE_BUTTON:
		gui_sync_button(elem);
		break;
	case GUI_TYPE_LABEL:
		gui_sync_label(elem);
		break;
	case GUI_TYPE_TEXT_BOX:
		gui_sync_text_box(elem);
		break;
	case GUI_TYPE_TEXT_INPUT:
		gui_sync_text_input(elem);
		break;
	case GUI_TYPE_CONTAINER:
		gui_sync_container(elem);
		break;
	default:
		break;
	}
}

void gui_sync(struct element *elem)
{
	gui_only_sync(elem);
	merge_elements(get_root(elem->window_id));
	gfx_redraw_focused();
}

void gui_sync_recursive(struct element *container)
{
	if (!container || !container->childs || !container->childs->head)
		return;

	struct node *iterator = container->childs->head;
	while (iterator != NULL) {
		gui_only_sync(iterator->data);
		gui_sync_recursive(iterator->data);
		iterator = iterator->next;
	}
}

void gui_sync_window(u32 window_id)
{
	struct element *root = get_root(window_id);

	if (!root || !root->childs || !root->childs->head)
		return;

	struct node *iterator = root->childs->head;
	while (iterator != NULL) {
		gui_only_sync(iterator->data);
		gui_sync_recursive(iterator->data);
		iterator = iterator->next;
	}

	merge_elements(root);
	gfx_redraw_focused();
}

struct element *gui_add_button(struct element *container, int x, int y, enum font_type font_type,
			       const char *text, u32 color_bg, u32 color_fg)
{
	if (!container || !container->childs || !gfx_resolve_font(font_type))
		return NULL;

	struct element *button = malloc(sizeof(*button));
	button->type = GUI_TYPE_BUTTON;
	button->window_id = container->window_id;
	button->ctx = NULL;
	button->parent = container;
	button->childs = list_new();
	button->data = malloc(sizeof(struct element_button));
	((struct element_button *)button->data)->x = x;
	((struct element_button *)button->data)->y = y;
	((struct element_button *)button->data)->text = strdup(text);
	((struct element_button *)button->data)->color_fg = color_fg;
	((struct element_button *)button->data)->color_bg = color_bg;
	((struct element_button *)button->data)->font_type = font_type;

	list_add(container->childs, button);
	gui_sync(button);

	return button;
}

struct element *gui_add_label(struct element *container, int x, int y, enum font_type font_type,
			      const char *text, u32 color_bg, u32 color_fg)
{
	if (!container || !container->childs || !gfx_resolve_font(font_type))
		return NULL;

	struct element *label = malloc(sizeof(*label));
	label->type = GUI_TYPE_LABEL;
	label->window_id = container->window_id;
	label->ctx = NULL;
	label->parent = container;
	label->childs = list_new();
	label->data = malloc(sizeof(struct element_label));
	((struct element_label *)label->data)->x = x;
	((struct element_label *)label->data)->y = y;
	((struct element_label *)label->data)->text = strdup(text);
	((struct element_label *)label->data)->color_fg = color_fg;
	((struct element_label *)label->data)->color_bg = color_bg;
	((struct element_label *)label->data)->font_type = font_type;

	list_add(container->childs, label);
	gui_sync(label);

	return label;
}

struct element *gui_add_text_box(struct element *container, int x, int y, u32 width, u32 height,
				 enum font_type font_type, const char *text, u32 color_bg,
				 u32 color_fg)
{
	if (!container || !container->childs || !gfx_resolve_font(font_type))
		return NULL;

	struct element *text_box = malloc(sizeof(*text_box));
	text_box->type = GUI_TYPE_TEXT_BOX;
	text_box->window_id = container->window_id;
	text_box->ctx = NULL;
	text_box->parent = container;
	text_box->childs = list_new();
	text_box->data = malloc(sizeof(struct element_text_box));
	((struct element_text_box *)text_box->data)->x = x;
	((struct element_text_box *)text_box->data)->y = y;
	((struct element_text_box *)text_box->data)->width = width;
	((struct element_text_box *)text_box->data)->height = height;
	((struct element_text_box *)text_box->data)->text = strdup(text);
	((struct element_text_box *)text_box->data)->color_fg = color_fg;
	((struct element_text_box *)text_box->data)->color_bg = color_bg;
	((struct element_text_box *)text_box->data)->font_type = font_type;

	list_add(container->childs, text_box);
	gui_sync(text_box);

	return text_box;
}

struct element *gui_add_text_input(struct element *container, int x, int y, u32 width,
				   enum font_type font_type, u32 color_bg, u32 color_fg)
{
	if (!container || !container->childs || !gfx_resolve_font(font_type))
		return NULL;

	struct element *text_input = malloc(sizeof(*text_input));
	text_input->type = GUI_TYPE_TEXT_INPUT;
	text_input->window_id = container->window_id;
	text_input->ctx = NULL;
	text_input->parent = container;
	text_input->childs = list_new();
	text_input->data = malloc(sizeof(struct element_text_input));
	((struct element_text_input *)text_input->data)->x = x;
	((struct element_text_input *)text_input->data)->y = y;
	((struct element_text_input *)text_input->data)->width = width;
	((struct element_text_input *)text_input->data)->color_fg = color_fg;
	((struct element_text_input *)text_input->data)->color_bg = color_bg;
	((struct element_text_input *)text_input->data)->font_type = font_type;

	list_add(container->childs, text_input);
	gui_sync(text_input);

	return text_input;
}

struct element *gui_add_container(struct element *container, int x, int y, u32 width, u32 height,
				  u32 color_bg)
{
	if (!container || !container->childs)
		return NULL;

	struct element *new_container = malloc(sizeof(*new_container));
	new_container->type = GUI_TYPE_CONTAINER;
	new_container->window_id = container->window_id;
	new_container->ctx = NULL;
	new_container->parent = container;
	new_container->childs = list_new();
	new_container->data = malloc(sizeof(struct element_container));
	((struct element_container *)new_container->data)->x = x;
	((struct element_container *)new_container->data)->y = y;
	((struct element_container *)new_container->data)->width = width;
	((struct element_container *)new_container->data)->height = height;
	((struct element_container *)new_container->data)->color_bg = color_bg;
	((struct element_container *)new_container->data)->flags = 0;

	list_add(container->childs, new_container);
	gui_sync(new_container);

	return new_container;
}

void gui_remove_childs(struct element *elem)
{
	remove_childs(elem);
	elem->childs = list_new();
	gui_sync(elem);
}

void gui_remove_element(struct element *elem)
{
	if (!elem)
		return;

	u32 id = elem->window_id;
	struct element *root = get_root(id);
	u8 is_root = root == elem;
	remove_element(elem);
	elem = NULL;
	if (!is_root) {
		merge_elements(get_root(id));
		gfx_redraw_focused();
	}
}

// TODO: Split into small functions
void gui_event_loop(struct element *container)
{
	if (!container)
		return;

	struct message *msg;
	struct element *focused = NULL;
	while (1) {
		if (!(msg = msg_receive())) {
			yield();
			continue;
		}

		switch (msg->type) {
		case GUI_KILL: {
			remove_all();
			exit(0);
		}
		case GUI_MOUSE: {
			struct gui_event_mouse *event = msg->data;
			focused = element_at(container, event->x, event->y);
			if (focused && focused->event.on_click && event->but1)
				focused->event.on_click(event, focused);
			break;
		}
		case GUI_KEYBOARD: {
			struct gui_event_keyboard *event = msg->data;

			if (focused && focused->type == GUI_TYPE_TEXT_INPUT && event->press) {
				char *s = ((struct element_text_input *)focused->data)->text;
				u32 l = strlen(s);
				if (event->ch >= ' ') {
					if (l >= MAX_INPUT_LENGTH)
						continue;
					s[l] = event->ch;
					s[l + 1] = '\0';
					gui_sync(focused);
				} else if (event->scancode == KEY_BACKSPACE && l > 0) {
					s[l - 1] = '\0';
					gui_sync(focused);
				}
			}

			if (focused && focused->event.on_submit && event->press &&
			    event->scancode == KEY_ENTER) {
				focused->event.on_submit(event, focused);
				// Clear!
				char *t = ((struct element_text_input *)focused->data)->text;
				memset(t, 0, strlen(t));
				gui_sync(focused);
			}

			if (focused && focused->event.on_key && event->press && event->ch)
				focused->event.on_key(event, focused);

			break;
		}
		case GUI_RESIZE: {
			struct gui_event_resize *event = msg->data;
			struct element *root = get_root(container->window_id);
			root->ctx = event->new_ctx;
			gui_sync_window(container->window_id);
			break;
		}
		}
	}

	exit(1);
}

struct element *gui_init(const char *title, u32 width, u32 height, u32 color_bg)
{
	if (window_count != 0)
		return NULL;

	// TODO: Add center flag
	struct window *win =
		new_window(title, 30, 30, width + BORDER * 2, height + BORDER * 2, WF_DEFAULT);
	if (!win)
		return NULL;

	gfx_fill(win->ctx, color_bg);

	struct element *container = malloc(sizeof(*container));
	container->type = GUI_TYPE_ROOT;
	container->window_id = win->id;
	container->ctx = win->ctx;
	container->childs = list_new();
	container->data = NULL;
	list_add(win->childs, container);

	struct element *root = gui_add_container(container, BORDER, BORDER, 100, 100, COLOR_BLACK);
	if (!root)
		return NULL;

	return root;
}

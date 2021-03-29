// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <def.h>
#include <errno.h>
#include <libgui/gui.h>
#include <libgui/msg.h>
#include <list.h>
#include <print.h>

#define WM_PATH "/bin/wm"

struct gui_widget {
	u32 id;
	vec2 pos;
	struct context ctx;
	struct list *children;

	struct {
		void (*mousemove)(u32 widget_id, vec2 pos);
		void (*mouseclick)(u32 widget_id, vec2 pos);
	} event;
};

struct gui_window {
	u32 id;
	struct context ctx;
	struct list *widgets;
};

struct list *windows = NULL;

/**
 * Resolve/find stuff
 */

static struct gui_window *gui_window_by_id(u32 win_id)
{
	assert(windows);
	struct node *iterator = windows->head;
	while (iterator) {
		struct gui_window *win = iterator->data;
		if (win->id == win_id)
			return iterator->data;
		iterator = iterator->next;
	}

	return NULL;
}

static struct gui_widget *gui_widget_in_widget(struct gui_widget *parent, u32 widget_id)
{
	if (!parent)
		return NULL;

	if (parent->id == widget_id)
		return parent;

	if (!parent->children)
		return NULL;

	struct node *iterator = parent->children->head;
	while (iterator) {
		struct gui_widget *widget = iterator->data;
		if (widget->id == widget_id)
			return iterator->data;

		struct gui_widget *sub = gui_widget_in_widget(widget, widget_id);
		if (sub && sub->id == widget_id)
			return sub;

		iterator = iterator->next;
	}

	return NULL;
}

static struct gui_widget *gui_widget_in_win(struct gui_window *win, u32 widget_id)
{
	if (!win || !win->widgets)
		return NULL;

	struct node *iterator = win->widgets->head;
	while (iterator) {
		struct gui_widget *widget = iterator->data;
		if (widget->id == widget_id)
			return iterator->data;

		struct gui_widget *sub = gui_widget_in_widget(widget, widget_id);
		if (sub && sub->id == widget_id)
			return sub;

		iterator = iterator->next;
	}

	return NULL;
}

static struct gui_widget *gui_widget_by_id(u32 win_id, u32 widget_id)
{
	struct gui_window *win = gui_window_by_id(win_id);
	return gui_widget_in_win(win, widget_id);
}

/**
 * GFX wrappers
 */

res gui_fill(u32 win_id, u32 widget_id, u32 c)
{
	struct gui_widget *widget = gui_widget_by_id(win_id, widget_id);
	if (!widget)
		return_errno(ENOENT);

	gfx_fill(&widget->ctx, c);

	return_errno(EOK);
}

/**
 * Widgets
 */

static res gui_sub_widget_at(struct gui_widget *widget, vec2 pos, struct gui_widget *buf)
{
	if (!widget || !widget->children || !buf)
		return_errno(EFAULT);

	struct gui_widget *ret = NULL;
	struct gui_widget sub = { 0 };

	struct node *iterator = widget->children->head;
	while (iterator) {
		struct gui_widget *w = iterator->data;
		if (pos.x >= w->pos.x && pos.x <= w->pos.x + w->ctx.size.x && pos.y >= w->pos.y &&
		    pos.y <= w->pos.y + w->ctx.size.y)
			ret = w;

		if (w->children->head) {
			if (gui_sub_widget_at(w, pos, &sub) == EOK)
				ret = &sub;
		}
		iterator = iterator->next;
	}

	if (ret) {
		*buf = *ret;
		return_errno(EOK);
	} else {
		return_errno(ENOENT);
	}
}

static res gui_widget_at(u32 win_id, vec2 pos, struct gui_widget *widget)
{
	if (!widget)
		return_errno(EFAULT);

	struct gui_window *win = gui_window_by_id(win_id);
	if (!win)
		return_errno(ENOENT);

	if (!win->widgets)
		return_errno(EOK);

	struct gui_widget *ret = NULL;
	struct gui_widget sub = { 0 };

	struct node *iterator = win->widgets->head;
	while (iterator) {
		struct gui_widget *w = iterator->data;
		if (pos.x >= w->pos.x && pos.x <= w->pos.x + w->ctx.size.x && pos.y >= w->pos.y &&
		    pos.y <= w->pos.y + w->ctx.size.y)
			ret = w;

		if (w->children->head) {
			if (gui_sub_widget_at(w, pos, &sub) == EOK)
				ret = &sub;
		}
		iterator = iterator->next;
	}

	if (ret) {
		*widget = *ret;
		return_errno(EOK);
	} else {
		return_errno(ENOENT);
	}
}

static res gui_sync_sub_widgets(struct gui_widget *widget)
{
	if (!widget)
		return_errno(EFAULT);

	if (!widget->children)
		return_errno(EOK);

	struct node *iterator = widget->children->head;
	while (iterator) {
		struct gui_widget *w = iterator->data;
		gfx_ctx_on_ctx(&widget->ctx, &w->ctx, w->pos);
		iterator = iterator->next;
	}

	return_errno(EOK);
}

static res gui_sync_widget(u32 win_id, u32 widget_id)
{
	struct gui_window *win = gui_window_by_id(win_id);
	struct gui_widget *widget = gui_widget_in_win(win, widget_id);
	if (!widget)
		return_errno(ENOENT);

	gui_sync_sub_widgets(widget);
	gfx_ctx_on_ctx(&win->ctx, &widget->ctx, widget->pos);

	return_errno(EOK);
}

static res gui_sync_widgets(u32 win_id)
{
	struct gui_window *win = gui_window_by_id(win_id);
	if (!win)
		return_errno(ENOENT);

	if (!win->widgets)
		return_errno(EOK);

	struct node *iterator = win->widgets->head;
	while (iterator) {
		struct gui_widget *widget = iterator->data;
		gui_sync_sub_widgets(widget);
		gfx_ctx_on_ctx(&win->ctx, &widget->ctx, widget->pos);
		iterator = iterator->next;
	}

	return_errno(EOK);
}

static struct gui_widget *gui_win_main_widget(struct gui_window *win)
{
	return win->widgets->head->data;
}

// TODO: This is very recursive and inefficient -> improve!
static vec2 gui_offset_widget(struct gui_widget *parent, struct gui_widget *child)
{
	if (!parent || !parent->children || !child)
		return vec2(0, 0);

	vec2 offset = vec2(0, 0);

	struct node *iterator = parent->children->head;
	while (iterator) {
		struct gui_widget *w = iterator->data;
		if (w == child) {
			offset = vec2_add(offset, w->pos);
			break;
		}
		struct gui_widget *sub = gui_widget_in_widget(w, child->id);
		if (sub) {
			offset = vec2_add(offset, w->pos);
			gui_offset_widget(w, child);
			break;
		}

		iterator = iterator->next;
	}

	return offset;
}

static struct gui_widget *gui_new_plain_widget(vec2 size, vec2 pos, u8 bpp)
{
	struct gui_widget *widget = zalloc(sizeof(*widget));
	struct context *ctx = zalloc(sizeof(*ctx));

	static u32 id = 0;
	widget->id = id++;
	widget->pos = pos;
	widget->ctx = *gfx_new_ctx(ctx, size, bpp);
	widget->children = list_new();

	return widget;
}

res gui_add_widget(u32 win_id, u32 widget_id, vec2 size, vec2 pos)
{
	struct gui_widget *parent = gui_widget_by_id(win_id, widget_id);
	if (!parent)
		return_errno(ENOENT);

	struct gui_widget *child = gui_new_plain_widget(size, pos, parent->ctx.bpp);
	list_add(parent->children, child);

	return child->id;
}

res gui_new_widget(u32 win_id, vec2 size, vec2 pos)
{
	struct gui_window *win = gui_window_by_id(win_id);
	if (!win)
		return_errno(ENOENT);

	if (!win->widgets->head)
		list_add(win->widgets,
			 gui_new_plain_widget(win->ctx.size, vec2(0, 0), win->ctx.bpp));

	return gui_add_widget(win->id, gui_win_main_widget(win)->id, size, pos);
}

res gui_listen_widget(u32 win_id, u32 widget_id, enum gui_listener listener, u32 func)
{
	if (!func)
		return_errno(EFAULT);

	struct gui_widget *widget = gui_widget_by_id(win_id, widget_id);
	if (!widget)
		return_errno(ENOENT);

	switch (listener) {
	case GUI_LISTEN_MOUSEMOVE:
		widget->event.mousemove = (void (*)(u32, vec2))func;
		break;
	case GUI_LISTEN_MOUSECLICK:
		widget->event.mouseclick = (void (*)(u32, vec2))func;
		break;
	default:
		return_errno(ENOENT);
	}

	return_errno(EOK);
}

res gui_redraw_widget(u32 win_id, u32 widget_id)
{
	if (gui_sync_widget(win_id, widget_id) != EOK)
		return errno;

	if (gui_redraw_window(win_id) != EOK)
		return errno;

	return_errno(EOK);
}

/**
 * Window data getters
 */

vec2 gui_window_size(u32 win_id)
{
	struct gui_window *win = gui_window_by_id(win_id);
	if (!win)
		return vec2(0, 0);
	return win->ctx.size;
}

/**
 * Window manager interfaces
 */

res gui_new_window(void)
{
	if (!windows)
		windows = list_new();

	struct gui_window *win = zalloc(sizeof(*win));

	struct message_new_window msg = { .header.state = MSG_NEED_ANSWER };
	if (msg_send(pidof(WM_PATH), GUI_NEW_WINDOW, &msg, sizeof(msg)) > 0 &&
	    msg_receive(&msg, sizeof(msg)) > 0 &&
	    msg.header.type == (GUI_NEW_WINDOW | MSG_SUCCESS)) {
		win->id = msg.id;
		win->ctx = msg.ctx;
		u32 size;
		res ret = shaccess(msg.shid, (u32 *)&win->ctx.fb, &size);
		if (ret < 0 || !win->ctx.fb)
			return_errno(-MIN(ret, -EFAULT));
		list_add(windows, win);
		win->widgets = list_new();

		return win->id;
	}

	return_errno(EINVAL);
}

res gui_redraw_window(u32 id)
{
	res ret = gui_sync_widgets(id);
	if (ret != 0)
		return_errno(ENOENT);

	struct message_redraw_window msg = { .id = id, .header.state = MSG_NEED_ANSWER };
	if (msg_send(pidof(WM_PATH), GUI_REDRAW_WINDOW, &msg, sizeof(msg)) > 0 &&
	    msg_receive(&msg, sizeof(msg)) > 0 &&
	    msg.header.type == (GUI_REDRAW_WINDOW | MSG_SUCCESS))
		return EOK;

	return_errno(EINVAL);
}

/**
 * Message handling
 */

static res gui_handle_error(const char *op, res code)
{
	log("GUI error at '%s': %s (%d)\n", op, strerror(code), code);
	return code;
}

static res gui_handle_ping(struct message_ping *msg)
{
	if (msg->ping != MSG_PING_SEND)
		return gui_handle_error("ping", EINVAL);

	msg->header.type |= MSG_SUCCESS;
	msg->ping = MSG_PING_RECV;
	msg_send(msg->header.src, GUI_PING, &msg, sizeof(msg));

	return errno;
}

static res gui_handle_mouse(struct message_mouse *msg)
{
	if (msg->header.state == MSG_NEED_ANSWER) {
		msg_send(msg->header.src, msg->header.type | MSG_SUCCESS, msg, sizeof(*msg));

		if (errno != EOK)
			return errno;
	}

	struct gui_widget widget = { 0 };
	if (gui_widget_at(msg->id, msg->pos, &widget) != EOK)
		return_errno(EOK);

	struct gui_window *win = gui_window_by_id(msg->id);
	vec2 offset = gui_offset_widget(gui_win_main_widget(win), &widget);

	if (widget.event.mousemove)
		widget.event.mousemove(widget.id, vec2_sub(msg->pos, offset));

	if (widget.event.mouseclick && msg->bits.click)
		widget.event.mouseclick(widget.id, vec2_sub(msg->pos, offset));

	return_errno(EOK);
}

static void gui_handle_exit(void)
{
	if (!windows)
		return;

	struct node *iterator = windows->head;
	while (iterator) {
		struct gui_window *win = iterator->data;
		struct message_destroy_window msg = { .id = win->id };
		msg_send(pidof(WM_PATH), GUI_DESTROY_WINDOW, &msg, sizeof(msg));
		iterator = iterator->next;
	}

	list_destroy(windows);
}

/**
 * Main loop
 */

void gui_loop(void)
{
	atexit(gui_handle_exit);

	if (!windows)
		err(1, "Create some windows first\n");

	void *msg = zalloc(4096);
	while (msg_receive(msg, 4096)) {
		struct message_header *head = msg;
		switch (head->type) {
		case GUI_PING:
			gui_handle_ping(msg);
			break;
		case GUI_MOUSE:
			gui_handle_mouse(msg);
			break;
		default:
			// TODO: Fix random unknown msg types
			gui_handle_error("loop", EINVAL);
		}
	}
}

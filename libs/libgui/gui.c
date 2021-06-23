// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <def.h>
#include <errno.h>
#include <libgui/gui.h>
#include <libgui/msg.h>
#include <list.h>
#include <print.h>

struct gui_widget {
	u32 id;
	vec2 pos;
	struct gfx_context fg;
	struct gfx_context bg;
	struct list *children;

	u32 margin; // Between sub-widgets
	enum gui_layout layout;

	struct {
		void (*mousemove)(struct gui_event_mouse *event);
		void (*mouseclick)(struct gui_event_mouse *event);
	} event;
};

struct gui_window {
	u32 id;
	vec2 off; // fb offset
	vec2 pos;
	struct gfx_context ctx;
	struct list *widgets;
};

struct list *windows = NULL;

#define gui_error(error)                                                                           \
	err(1, "%s:%d in %s: GUI Error: %s\n", __FILE__, __LINE__, __func__, strerror(error))

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
 * Bus stuff
 */

static u32 wm_conn = 0;
static void gui_connect_wm(void)
{
	if (wm_conn) {
		assert(msg_connect_conn(wm_conn) == EOK);
	} else {
		assert(msg_connect_bus("wm", &wm_conn) == EOK);
	}
}

/**
 * GFX wrappers
 */

void gui_clear(u32 win_id, u32 widget_id, enum gui_layer layer)
{
	struct gui_widget *widget = gui_widget_by_id(win_id, widget_id);
	if (!widget)
		gui_error(ENOENT);

	if (layer == GUI_LAYER_BG)
		gfx_clear(&widget->bg);
	else if (layer == GUI_LAYER_FG)
		gfx_clear(&widget->fg);
	else
		gui_error(EINVAL);
}

void gui_fill(u32 win_id, u32 widget_id, enum gui_layer layer, u32 c)
{
	struct gui_widget *widget = gui_widget_by_id(win_id, widget_id);
	if (!widget)
		gui_error(ENOENT);

	if (layer == GUI_LAYER_BG)
		gfx_fill(&widget->bg, c);
	else if (layer == GUI_LAYER_FG)
		gfx_fill(&widget->fg, c);
	else
		gui_error(EINVAL);
}

void gui_write(u32 win_id, u32 widget_id, enum gui_layer layer, vec2 pos, enum font_type font_type,
	       u32 c, const char *text)
{
	struct gui_widget *widget = gui_widget_by_id(win_id, widget_id);
	if (!widget)
		gui_error(ENOENT);

	if (layer == GUI_LAYER_BG)
		gfx_write(&widget->bg, pos, font_type, c, text);
	else if (layer == GUI_LAYER_FG)
		gfx_write(&widget->fg, pos, font_type, c, text);
	else
		gui_error(EINVAL);
}

void gui_draw_image_filter(u32 win_id, u32 widget_id, enum gui_layer layer, vec2 pos, vec2 size,
			   enum gfx_filter filter, const char *path)
{
	UNUSED(size); // TODO: Add image scaling

	struct gui_widget *widget = gui_widget_by_id(win_id, widget_id);
	if (!widget)
		gui_error(ENOENT);

	if (layer == GUI_LAYER_BG)
		gfx_draw_image_filter(&widget->bg, pos, size, filter, path);
	else if (layer == GUI_LAYER_FG)
		gfx_draw_image_filter(&widget->fg, pos, size, filter, path);
	else
		gui_error(EINVAL);
}

void gui_draw_image(u32 win_id, u32 widget_id, enum gui_layer layer, vec2 pos, vec2 size,
		    const char *path)
{
	gui_draw_image_filter(win_id, widget_id, layer, pos, size, GFX_FILTER_NONE, path);
}

void gui_draw_rectangle(u32 win_id, u32 widget_id, enum gui_layer layer, vec2 pos1, vec2 pos2,
			u32 c)
{
	struct gui_widget *widget = gui_widget_by_id(win_id, widget_id);
	if (!widget)
		gui_error(ENOENT);

	if (layer == GUI_LAYER_BG)
		gfx_draw_rectangle(&widget->bg, pos1, pos2, c);
	else if (layer == GUI_LAYER_FG)
		gfx_draw_rectangle(&widget->fg, pos1, pos2, c);
	else
		gui_error(EINVAL);
}

void gui_draw_border(u32 win_id, u32 widget_id, enum gui_layer layer, u32 width, u32 c)
{
	struct gui_widget *widget = gui_widget_by_id(win_id, widget_id);
	if (!widget)
		gui_error(ENOENT);

	if (layer == GUI_LAYER_BG)
		gfx_draw_border(&widget->bg, width, c);
	else if (layer == GUI_LAYER_FG)
		gfx_draw_border(&widget->fg, width, c);
	else
		gui_error(EINVAL);
}

void gui_draw_line(u32 win_id, u32 widget_id, enum gui_layer layer, vec2 pos1, vec2 pos2, u32 scale,
		   u32 c)
{
	struct gui_widget *widget = gui_widget_by_id(win_id, widget_id);
	if (!widget)
		gui_error(ENOENT);

	if (layer == GUI_LAYER_BG)
		gfx_draw_line(&widget->bg, pos1, pos2, scale, c);
	else if (layer == GUI_LAYER_FG)
		gfx_draw_line(&widget->fg, pos1, pos2, scale, c);
	else
		gui_error(EINVAL);
}

/**
 * Widgets
 */

static u8 gui_sub_widget_at(struct gui_widget *widget, vec2 pos, struct gui_widget *buf)
{
	if (!widget || !widget->children || !buf)
		gui_error(EFAULT);

	struct gui_widget *ret = NULL;
	struct gui_widget sub = { 0 };

	struct node *iterator = widget->children->head;
	while (iterator) {
		struct gui_widget *w = iterator->data;
		if (pos.x >= w->pos.x && pos.x <= w->pos.x + w->bg.size.x && pos.y >= w->pos.y &&
		    pos.y <= w->pos.y + w->bg.size.y)
			ret = w;

		if (w->children->head && gui_sub_widget_at(w, pos, &sub))
			ret = &sub;
		iterator = iterator->next;
	}

	if (!ret)
		return 0;

	*buf = *ret;
	return 1;
}

static u8 gui_widget_at(u32 win_id, vec2 pos, struct gui_widget *widget)
{
	if (!widget)
		gui_error(EFAULT);

	struct gui_window *win = gui_window_by_id(win_id);
	if (!win)
		gui_error(ENOENT);

	if (!win->widgets)
		return 1;

	struct gui_widget *ret = NULL;
	struct gui_widget sub = { 0 };

	struct node *iterator = win->widgets->head;
	while (iterator) {
		struct gui_widget *w = iterator->data;
		if (pos.x >= w->pos.x && pos.x <= w->pos.x + w->bg.size.x && pos.y >= w->pos.y &&
		    pos.y <= w->pos.y + w->bg.size.y)
			ret = w;

		if (w->children->head && gui_sub_widget_at(w, pos, &sub))
			ret = &sub;
		iterator = iterator->next;
	}

	if (!ret)
		return 0;

	*widget = *ret;
	return 1;
}

static void gui_sync_sub_widgets(struct gui_widget *widget)
{
	if (!widget)
		gui_error(EFAULT);

	if (!widget->children)
		return;

	struct node *iterator = widget->children->head;
	while (iterator) {
		struct gui_widget *w = iterator->data;
		gui_sync_sub_widgets(w);
		gfx_ctx_on_ctx(&widget->bg, &w->bg, w->pos, GFX_NON_ALPHA);
		gfx_ctx_on_ctx(&widget->fg, &w->fg, w->pos, GFX_NON_ALPHA);
		iterator = iterator->next;
	}
}

static void gui_sync_widget(u32 win_id, u32 widget_id)
{
	struct gui_window *win = gui_window_by_id(win_id);
	struct gui_widget *widget = gui_widget_in_win(win, widget_id);
	if (!widget)
		gui_error(ENOENT);

	gui_sync_sub_widgets(widget);
	gfx_ctx_on_ctx(&win->ctx, &widget->bg, widget->pos, GFX_ALPHA);
	gfx_ctx_on_ctx(&win->ctx, &widget->fg, widget->pos, GFX_ALPHA);
}

static void gui_sync_widgets(u32 win_id)
{
	struct gui_window *win = gui_window_by_id(win_id);
	if (!win)
		gui_error(ENOENT);

	if (!win->widgets)
		return;

	struct node *iterator = win->widgets->head;
	while (iterator) {
		struct gui_widget *widget = iterator->data;
		gui_sync_sub_widgets(widget);
		gfx_ctx_on_ctx(&win->ctx, &widget->bg, widget->pos, GFX_ALPHA);
		gfx_ctx_on_ctx(&win->ctx, &widget->fg, widget->pos, GFX_ALPHA);
		iterator = iterator->next;
	}
}

static struct gui_widget *gui_main_widget(struct gui_window *win)
{
	assert(win && win->widgets && win->widgets->head);
	return win->widgets->head->data;
}

static u32 gui_main_widget_id(struct gui_window *win)
{
	assert(win && win->widgets && win->widgets->head);
	return ((struct gui_widget *)win->widgets->head->data)->id;
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

static struct gui_widget *gui_new_plain_widget(vec2 pos, vec2 size, enum gui_layout layout, u8 bpp)
{
	struct gui_widget *widget = zalloc(sizeof(*widget));
	struct gfx_context bg;
	struct gfx_context fg;

	static u32 id = 0;
	widget->id = id++;
	widget->pos = pos;
	widget->bg = *gfx_new_ctx(&bg, size, bpp);
	widget->fg = *gfx_new_ctx(&fg, size, bpp);
	widget->children = list_new();
	widget->margin = 4;
	widget->layout = layout;

	return widget;
}

u32 gui_widget(u32 win_id, u32 widget_id, vec2 pos, vec2 size)
{
	struct gui_widget *parent = gui_widget_by_id(win_id, widget_id);
	if (!parent)
		gui_error(ENOENT);

	struct gui_widget *child = gui_new_plain_widget(pos, size, GUI_HLAYOUT, parent->bg.bpp);
	list_add(parent->children, child);

	return child->id;
}

static void gui_destroy_widget(u32 win_id, u32 widget_id)
{
	struct gui_widget *widget = gui_widget_by_id(win_id, widget_id);
	if (!widget)
		return;

	struct node *iterator = widget->children->head;
	while (iterator) {
		struct gui_widget *sub = iterator->data;
		gui_destroy_widget(win_id, sub->id);
		iterator = iterator->next;
	}

	list_destroy(widget->children);
	free(widget->bg.fb);
	free(widget->fg.fb);
	free(widget);
}

static void gui_destroy_widgets(u32 win_id)
{
	struct gui_window *win = gui_window_by_id(win_id);

	if (!win->widgets)
		return;

	struct node *iterator = win->widgets->head;
	while (iterator) {
		struct gui_widget *widget = iterator->data;
		gui_destroy_widget(win_id, widget->id);
		iterator = iterator->next;
	}
	list_destroy(win->widgets);
}

void gui_widget_listen(u32 win_id, u32 widget_id, enum gui_listener listener, u32 func)
{
	if (!func)
		gui_error(EFAULT);

	struct gui_widget *widget = gui_widget_by_id(win_id, widget_id);
	if (!widget)
		gui_error(ENOENT);

	switch (listener) {
	case GUI_LISTEN_MOUSEMOVE:
		widget->event.mousemove = (void (*)(struct gui_event_mouse *))func;
		break;
	case GUI_LISTEN_MOUSECLICK:
		widget->event.mouseclick = (void (*)(struct gui_event_mouse *))func;
		break;
	default:
		gui_error(ENOENT);
	}
}

void gui_redraw_widget(u32 win_id, u32 widget_id)
{
	gui_sync_widget(win_id, widget_id);
	gui_redraw_window_only(win_id);
}

/**
 * Popups/alerts
 */

#define POPUP_WIDTH 200
#define POPUP_HEIGHT 100

void gui_popup(const char *text)
{
	vec2 pos = vec2(200, 200);

	u32 popup = gui_custom_window("Popup", pos, vec2(POPUP_WIDTH, POPUP_HEIGHT));
	struct gui_window *win = gui_window_by_id(popup);
	gui_fill(popup, gui_main_widget_id(win), GUI_LAYER_BG, COLOR_WHITE);

	u32 widget = gui_widget(popup, GUI_MAIN, vec2(0, 0), vec2(POPUP_WIDTH, 32));
	gui_fill(popup, widget, GUI_LAYER_BG, COLOR_WHITE);
	gui_write(popup, widget, GUI_LAYER_FG, vec2(0, 0), FONT_32, COLOR_BLACK, text);

	gui_redraw_window(popup);
}

/**
 * Window data getters
 */

vec2 gui_window_size(u32 win_id)
{
	struct gui_window *win = gui_window_by_id(win_id);
	if (!win)
		gui_error(ENOENT);
	return win->ctx.size;
}

/**
 * Window manager interfaces
 */

u32 gui_custom_window(const char *name, vec2 pos, vec2 size)
{
	if (!windows)
		windows = list_new();

	if (vec2_sum(pos) == 0)
		pos = vec2(200, 200);

	if (vec2_sum(size) == 0)
		size = vec2(600, 400);

	struct gui_window *win = zalloc(sizeof(*win));
	struct message_new_window msg = { .header.state = MSG_NEED_ANSWER,
					  .pos = pos,
					  .size = size };
	strlcpy(msg.name, name, sizeof(msg.name));

	gui_connect_wm();
	if (msg_send(GUI_NEW_WINDOW, &msg, sizeof(msg)) > 0 && msg_receive(&msg, sizeof(msg)) > 0 &&
	    msg.header.type == (GUI_NEW_WINDOW | MSG_SUCCESS)) {
		win->id = msg.id;
		win->off = msg.off;
		win->ctx = msg.ctx;
		win->pos = msg.pos;
		u32 buf_size;
		res ret = shaccess(msg.shid, (u32 *)&win->ctx.fb, &buf_size);
		if (ret < 0 || !win->ctx.fb)
			gui_error(-MIN(ret, -EFAULT));

		// Apply offset
		win->ctx.size = vec2_sub(win->ctx.size, msg.off);
		win->ctx.bytes -= msg.off.y * msg.ctx.pitch;
		win->ctx.fb += msg.off.y * msg.ctx.pitch;

		list_add(windows, win);
		win->widgets = list_new();

		// Initialize GUI_MAIN widget
		list_add(win->widgets, gui_new_plain_widget(vec2(0, 0), win->ctx.size, GUI_HLAYOUT,
							    win->ctx.bpp));

		return win->id;
	}

	gui_error(EINVAL);
}

u32 gui_window(const char *name)
{
	return gui_custom_window(name, vec2(0, 0), vec2(0, 0));
}

void gui_redraw_window_only(u32 id)
{
	struct message_redraw_window msg = { .id = id };
	gui_connect_wm();
	if (msg_send(GUI_REDRAW_WINDOW, &msg, sizeof(msg)) > 0)
		return;

	gui_error(EINVAL);
}

void gui_redraw_window(u32 id)
{
	gui_sync_widgets(id);
	gui_redraw_window_only(id);
}

static void gui_destroy_window(u32 id)
{
	gui_destroy_widgets(id);

	struct gui_window *win = gui_window_by_id(id);
	u8 *fb = win->ctx.fb - (win->off.y * win->ctx.pitch);
	assert(sys_free(fb) == EOK);

	struct message_destroy_window msg = { .id = id };
	gui_connect_wm();
	msg_send(GUI_DESTROY_WINDOW, &msg, sizeof(msg));

	list_remove(windows, list_first_data(windows, win));
	free(win);
}

static void gui_destroy_windows(void)
{
	struct node *iterator = windows->head;
	while (iterator) {
		struct gui_window *win = iterator->data;
		iterator = iterator->next;
		gui_destroy_window(win->id);
	}

	list_destroy(windows);
}

/**
 * Message handling
 */

static void gui_handle_ping_window(struct message_ping_window *msg)
{
	if (msg->ping != MSG_PING_SEND)
		gui_error(EINVAL);

	msg->ping = MSG_PING_RECV;
	if (msg_connect_conn(msg->header.bus.conn) == EOK &&
	    msg_send(msg->header.type | MSG_SUCCESS, msg, sizeof(*msg)) > 0)
		return;
	gui_error(EINVAL);
}

static void gui_handle_mouse(struct message_mouse *msg)
{
	if (msg->header.state == MSG_NEED_ANSWER) {
		if (msg_connect_conn(msg->header.bus.conn) == EOK &&
		    msg_send(msg->header.type | MSG_SUCCESS, msg, sizeof(*msg)) > 0)
			return;
		gui_error(EINVAL);
	}

	struct gui_widget widget = { 0 };
	gui_widget_at(msg->id, msg->pos, &widget);

	struct gui_window *win = gui_window_by_id(msg->id);
	vec2 offset = gui_offset_widget(gui_main_widget(win), &widget);
	vec2 pos = vec2_sub(msg->pos, offset);

	struct gui_event_mouse event = {
		.win = win->id,
		.widget = widget.id,
		.pos = pos,
		.scroll = msg->scroll,
		.but.left = msg->but.left,
		.but.right = msg->but.right,
		.but.middle = msg->but.middle,
	};

	if (widget.event.mousemove)
		widget.event.mousemove(&event);

	if (widget.event.mouseclick && msg->but.left)
		widget.event.mouseclick(&event);
}

static void gui_handle_destroy_window(struct message_destroy_window *msg)
{
	gui_destroy_window(msg->id);
	if (!windows || !windows->head || !windows->head->data) {
		log("No more windows, exiting\n");
		exit(0);
	}
}

static void gui_handle_exit(void)
{
	if (!windows)
		return;

	gui_destroy_windows();
}

/**
 * Main loop
 */

void gui_loop(void)
{
	atexit(gui_handle_exit);

	if (!windows)
		err(1, "Create some windows first\n");

	u8 msg[4096] = { 0 };
	while (gui_connect_wm(), msg_receive(msg, 4096) > 0) {
		struct message_header *head = (void *)msg;
		switch (head->type) {
		case GUI_MOUSE:
			gui_handle_mouse((void *)msg);
			break;
		case GUI_PING_WINDOW:
			gui_handle_ping_window((void *)msg);
			break;
		case GUI_DESTROY_WINDOW:
			gui_handle_destroy_window((void *)msg);
			break;
		default:
			// TODO: Fix random unknown msg types
			gui_error(EINVAL);
		}
	}

	err(1, "Gui loop failed\n");
}

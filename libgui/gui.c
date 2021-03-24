// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <def.h>
#include <errno.h>
#include <gui.h>
#include <list.h>
#include <msg.h>
#include <print.h>

#define WM_PATH "/bin/wm"

struct gui_window {
	u32 id;
	struct context ctx;
};

struct list *windows = NULL;

static struct gui_window *win_by_id(u32 id)
{
	assert(windows);
	struct node *iterator = windows->head;
	while (iterator) {
		struct gui_window *win = iterator->data;
		if (win->id == id)
			return iterator->data;
		iterator = iterator->next;
	}
	return NULL;
}

/**
 * GFX wrappers
 */

static res fill(u32 id, u32 c)
{
	struct gui_window *win = win_by_id(id);
	if (!win)
		return -ENOENT;
	gfx_fill(&win->ctx, c);
	return EOK;
}

/**
 * Program interface
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
			return MIN(ret, -EFAULT);
		list_add(windows, win);
		assert(fill(win->id, COLOR_BLACK) == EOK);
		gui_redraw_window(win->id);
		return win->id;
	}
	return -EINVAL;
}

res gui_redraw_window(u32 id)
{
	struct message_redraw_window msg = { .id = id, .header.state = MSG_NEED_ANSWER };
	if (msg_send(pidof(WM_PATH), GUI_REDRAW_WINDOW, &msg, sizeof(msg)) > 0 &&
	    msg_receive(&msg, sizeof(msg)) > 0 &&
	    msg.header.type == (GUI_REDRAW_WINDOW | MSG_SUCCESS))
		return id;
	return -EINVAL;
}

/**
 * Message handling
 */

static res handle_error(const char *op, res code)
{
	log("GUI error at '%s': %s (%d)\n", op, strerror(code), code);
	return code;
}

static res handle_ping(struct message_ping *msg)
{
	if (msg->ping != MSG_PING_SEND)
		return handle_error("ping", EINVAL);

	msg->header.type |= MSG_SUCCESS;
	msg->ping = MSG_PING_RECV;
	msg_send(msg->header.src, GUI_PING, &msg, sizeof(msg));

	return errno;
}

static res handle_mouse(struct message_mouse *msg)
{
	if (msg->header.state == MSG_NEED_ANSWER)
		msg_send(msg->header.src, msg->header.type | MSG_SUCCESS, msg, sizeof(*msg));

	return errno;
}

static void handle_exit(void)
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
	atexit(handle_exit);

	void *msg = zalloc(4096);
	while (msg_receive(msg, 4096)) {
		struct message_header *head = msg;
		switch (head->type) {
		case GUI_PING:
			handle_ping(msg);
			break;
		case GUI_MOUSE:
			handle_mouse(msg);
			break;
		default:
			handle_error("loop", EINVAL);
		}
	}
}

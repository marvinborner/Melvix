// MIT License, Copyright (c) 2021 Marvin Borner

#include <def.h>
#include <errno.h>
#include <gui.h>
#include <msg.h>
#include <print.h>

#define WM_PATH "/bin/wm"

s32 gui_new_window(struct gui_window *win)
{
	struct message_new_window msg = { 0 };
	if (msg_send(pidof(WM_PATH), GUI_NEW_WINDOW, &msg, sizeof(msg)) > 0 &&
	    msg_receive(&msg, sizeof(msg)) > 0 &&
	    msg.header.type == (GUI_NEW_WINDOW | MSG_SUCCESS)) {
		win->id = msg.id;
		win->ctx = msg.ctx;
		u32 size;
		res ret = shaccess(msg.shid, (u32 *)&win->ctx.fb, &size);
		if (ret < 0 || !win->ctx.fb)
			return MIN(ret, -1);
		return win->id;
	}
	return -1;
}

s32 gui_redraw_window(u32 id)
{
	struct message_redraw_window msg = { .id = id };
	if (msg_send(pidof(WM_PATH), GUI_REDRAW_WINDOW, &msg, sizeof(msg)) > 0 &&
	    msg_receive(&msg, sizeof(msg)) > 0 &&
	    msg.header.type == (GUI_REDRAW_WINDOW | MSG_SUCCESS))
		return id;
	return -1;
}

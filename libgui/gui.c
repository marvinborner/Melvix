// MIT License, Copyright (c) 2021 Marvin Borner

#include <def.h>
#include <gui.h>
#include <print.h>

#define WM_PATH "/bin/wm"

s32 gui_new_window(struct gui_window *win)
{
	struct message msg = { 0 };
	if (msg_send(pidof(WM_PATH), GUI_NEW_WINDOW, win) > 0 && msg_receive(&msg) > 0 &&
	    msg.type == (GUI_NEW_WINDOW | MSG_SUCCESS))
		return win->id;
	return -1;
}

s32 gui_redraw_window(u32 id)
{
	struct message msg = { 0 };
	if (msg_send(pidof(WM_PATH), GUI_REDRAW_WINDOW, &id) > 0 && msg_receive(&msg) > 0 &&
	    msg.type == (GUI_REDRAW_WINDOW | MSG_SUCCESS))
		return id;
	return -1;
}

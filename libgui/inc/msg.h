// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef MSG_H
#define MSG_H

#include <def.h>
#include <gfx.h>

#define MSG_MAGIC 0x42042069
#define MSG_SUCCESS (1 << 29)
#define MSG_FAILURE (1 << 30)

struct message_header {
	u32 magic;
	u32 src;
	u32 type;
	u32 size;
};

struct message_new_window {
	struct message_header header;
	u32 id;
	struct context ctx;
};

struct message_redraw_window {
	struct message_header header;
	u32 id;
};

enum message_type {
	// GFX // TODO: Remove
	GFX_NEW_CONTEXT,
	GFX_REDRAW,
	GFX_REDRAW_FOCUSED,

	// GUI
	GUI_NEW_WINDOW,
	GUI_REDRAW_WINDOW,
	GUI_KILL,
	GUI_KEYBOARD,
	GUI_MOUSE,
	GUI_RESIZE,
	GUI_MAX
};

int msg_send(u32 pid, enum message_type type, void *data, u32 size);
int msg_receive(void *buf, u32 size);

#endif

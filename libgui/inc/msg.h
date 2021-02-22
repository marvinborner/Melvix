// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef MSG_H
#define MSG_H

#include <def.h>

#define MSG_MAGIC 0x42042069
#define MSG_SUCCESS (1 << 29)
#define MSG_FAILURE (1 << 30)

struct message {
	u32 magic;
	int src;
	int type;
	void *data;
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

int msg_send(u32 pid, enum message_type, void *data);
int msg_receive(struct message *msg);

#endif

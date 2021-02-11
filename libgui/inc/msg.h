// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef MSG_H
#define MSG_H

#include <def.h>

#define MSG_MAGIC 0x42042069

struct message {
	u32 magic;
	int src;
	int type;
	void *data;
};

enum message_type {
	// GFX
	GFX_NEW_CONTEXT,
	GFX_REDRAW,
	GFX_REDRAW_FOCUSED,

	// GUI
	GUI_KILL,
	GUI_KEYBOARD,
	GUI_MOUSE,
	GUI_RESIZE,
	GUI_MAX
};

int msg_send(u32 pid, enum message_type, void *data);
int msg_receive(struct message *msg);

#endif

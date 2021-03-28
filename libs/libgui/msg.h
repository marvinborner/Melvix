// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef MSG_H
#define MSG_H

#include <def.h>
#include <libgui/gfx.h>

#define MSG_PING_SEND 0x07734
#define MSG_PING_RECV 0x7474

#define MSG_MAGIC 0x42042069
#define MSG_SUCCESS (1 << 29)
#define MSG_FAILURE (1 << 30)

enum message_state {
	MSG_GO_ON,
	MSG_NEED_ANSWER,
};

struct message_header {
	u32 magic;
	u32 src;
	u32 type;
	enum message_state state;
};

struct message_ping {
	struct message_header header;
	u32 ping;
};

struct message_new_window {
	struct message_header header;
	u32 id;
	u32 shid;
	struct context ctx;
};

struct message_redraw_window {
	struct message_header header;
	u32 id;
};

struct message_destroy_window {
	struct message_header header;
	u32 id;
};

struct message_mouse {
	struct message_header header;
	u32 id;
	vec2 pos;
	struct {
		u8 click : 1;
	} bits;
};

enum message_type {
	GUI_PING,
	GUI_NEW_WINDOW,
	GUI_REDRAW_WINDOW,
	GUI_DESTROY_WINDOW,

	GUI_MOUSE,
	GUI_KEYBOARD,
};

res msg_send(u32 pid, enum message_type type, void *data, u32 size);
res msg_receive(void *buf, u32 size);

#endif
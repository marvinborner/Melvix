// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef SOCKET_H
#define SOCKET_H

#include <def.h>
#include <list.h>

// TODO: Use actual socket types (stream etc)
enum socket_type { S_TCP, S_UDP };
enum socket_state { S_CONNECTING, S_CONNECTED, S_OPEN, S_CLOSING, S_CLOSED, S_FAILED };

struct tcp_socket {
	u32 seq_no;
	u32 ack_no;
	u32 state;
};

struct socket_data {
	u8 *data;
	u32 length;
};

struct socket {
	u32 pid;
	u32 ip_addr;
	u32 dst_port;
	u32 src_port;
	enum socket_state state;
	enum socket_type type;
	struct list *packets;
	union {
		struct tcp_socket tcp;
		/* struct udp_socket udp; */
	} prot;
};

#endif

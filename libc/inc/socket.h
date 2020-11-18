// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef SOCKET_H
#define SOCKET_H

#include <def.h>

// TODO: Use actual socket types (stream etc)
enum socket_type { S_TCP, S_UDP };
enum socket_state { S_CONNECTING, S_CONNECTED, S_OPEN, S_CLOSED, S_FAILED };

struct tcp_socket {
	u32 seq_no;
	u32 ack_no;
	u32 state;
};

struct socket {
	u32 ip_addr;
	u32 dst_port;
	u32 src_port;
	enum socket_state state;
	enum socket_type type;
	u32 bytes_available;
	u32 bytes_read;
	void *current_packet;
	union {
		struct tcp_socket tcp;
		/* struct udp_socket udp; */
	} prot;
};

#endif

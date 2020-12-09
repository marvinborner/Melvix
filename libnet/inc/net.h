// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef NET_H
#define NET_H

#include <dns.h>
#include <ip.h>
#include <print.h>
#include <socket.h>
#include <sys.h>

#define htonl(l)                                                                                   \
	((((l)&0xff) << 24) | (((l)&0xff00) << 8) | (((l)&0xff0000) >> 8) |                        \
	 (((l)&0xff000000) >> 24))
#define htons(s) ((((s)&0xff) << 8) | (((s)&0xff00) >> 8))
#define ntohl(l) htonl((l))
#define ntohs(s) htons((s))
#define ip(a, b, c, d)                                                                             \
	((((a)&0xff) << 24) | (((b)&0xff) << 16) | (((c)&0xff) << 8) | (((d)&0xff) << 0))

#define NET_TIMEOUT 2000
#define NET_NO_TIMEOUT 0

static inline int net_data_available(struct socket *socket)
{
	return (socket && socket->packets && socket->packets->head && socket->packets->head->data &&
		((struct socket_data *)socket->packets->head->data)->length > 0);
}

#define net_open(type) (void *)sys1(SYS_NET_OPEN, (int)(type))
#define net_send(socket, data, len) (void)sys3(SYS_NET_SEND, (int)(socket), (int)(data), (int)(len))

static inline int net_connect(struct socket *socket, u32 ip_addr, u16 dst_port, u32 timeout)
{
	if (!socket || !ip_addr || !dst_port)
		return 0;
	sys3(SYS_NET_CONNECT, (int)(socket), (int)(ip_addr), (int)(dst_port));
	int time = time();
	while (socket->state != S_CONNECTED) {
		if (socket->state == S_FAILED || (timeout && time() - time >= timeout))
			return 0;
		yield();
	}
	return 1;
}

static inline int net_close(struct socket *socket)
{
	if (!socket)
		return 0;
	int res = 0;
	while (socket->state == S_CLOSING || !(res = (int)sys1(SYS_NET_CLOSE, (int)(socket))))
		yield();
	return res;
}

static inline int net_receive(struct socket *socket, void *buf, u32 len, u32 timeout)
{
	if (!socket || !buf || !len)
		return 0;

	int time = time();
	while (!net_data_available(socket)) {
		if (socket->state == S_FAILED || (timeout && time() - time >= timeout))
			return 0;
		yield();
	}

	// TODO: Only return once all segments are received?
	return (int)sys3(SYS_NET_RECEIVE, (int)(socket), (int)(buf), (int)(len));
}

#endif

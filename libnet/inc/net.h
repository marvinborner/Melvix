// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef NET_H
#define NET_H

#include <socket.h>

#define htonl(l)                                                                                   \
	((((l)&0xff) << 24) | (((l)&0xff00) << 8) | (((l)&0xff0000) >> 8) |                        \
	 (((l)&0xff000000) >> 24))
#define htons(s) ((((s)&0xff) << 8) | (((s)&0xff00) >> 8))
#define ntohl(l) htonl((l))
#define ntohs(s) htons((s))
#define ip(a, b, c, d)                                                                             \
	((((a)&0xff) << 24) | (((b)&0xff) << 16) | (((c)&0xff) << 8) | (((d)&0xff) << 0))

#define net_open(type) (void *)sys1(SYS_NET_OPEN, (int)(type))
#define net_close(socket) (void)sys1(SYS_NET_CLOSE, (int)(type))
#define net_connect(socket, ip_addr, dst_port)                                                     \
	(int)sys3(SYS_NET_CONNECT, (int)(socket), (int)(ip_addr), (int)(dst_port))
#define net_send(socket, data, len) (void)sys3(SYS_NET_SEND, (int)(socket), (int)(data), (int)(len))

void dns_request(const char *name, const char *tld);

#endif

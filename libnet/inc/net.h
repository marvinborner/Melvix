// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef NET_H
#define NET_H

#include <http.h>
#include <socket.h>
#include <sys.h>

#define DNS_NOERROR 0
#define DNS_FORMERR 1
#define DNS_SERVFAIL 2
#define DNS_NXDOMAIN 3
#define DNS_NOTIMP 4
#define DNS_REFUSED 5
#define DNS_YXDOMAIN 6
#define DNS_XRRSET 7
#define DNS_NOTAUTH 8
#define DNS_NOTZONE 9

#define htonl(l)                                                                                   \
	((((l)&0xff) << 24) | (((l)&0xff00) << 8) | (((l)&0xff0000) >> 8) |                        \
	 (((l)&0xff000000) >> 24))
#define htons(s) ((((s)&0xff) << 8) | (((s)&0xff00) >> 8))
#define ntohl(l) htonl((l))
#define ntohs(s) htons((s))
#define ip(a, b, c, d)                                                                             \
	((((a)&0xff) << 24) | (((b)&0xff) << 16) | (((c)&0xff) << 8) | (((d)&0xff) << 0))

#define net_open(type) (void *)sys1(SYS_NET_OPEN, (int)(type))
#define net_close(socket) (void)sys1(SYS_NET_CLOSE, (int)(socket))
#define net_connect(socket, ip_addr, dst_port)                                                     \
	(int)sys3(SYS_NET_CONNECT, (int)(socket), (int)(ip_addr), (int)(dst_port))
#define net_send(socket, data, len) (void)sys3(SYS_NET_SEND, (int)(socket), (int)(data), (int)(len))
static inline int net_receive(struct socket *socket, void *buf, u32 len)
{
	int res = 0;
	while ((res = (int)sys3(SYS_NET_RECEIVE, (int)(socket), (int)(buf), (int)(len))) == 0)
		;
	return res;
}

u32 dns_request(const char *name, const char *tld);

#endif

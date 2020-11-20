// MIT License, Copyright (c) 2020 Marvin Borner
// TODO: Less magic, auto xld splitting
// TODO: DNS cache

#include <def.h>
#include <mem.h>
#include <net.h>
#include <print.h>
#include <random.h>
#include <socket.h>
#include <str.h>

static u32 dns_ip_addr = ip(1, 1, 1, 1);

struct dns_packet {
	u16 qid;
	u16 flags;
	u16 questions;
	u16 answers;
	u16 authorities;
	u16 additional;
	u8 data[];
} __attribute__((packed));

static void dns_make_packet(struct dns_packet *packet, const char *name, const char *tld)
{
	packet->qid = htons(rand());
	packet->flags = htons(0x0100); // Standard query
	packet->questions = htons(1);
	packet->answers = htons(0);
	packet->authorities = htons(0);
	packet->additional = htons(0);

	packet->data[0] = (u8)strlen(name);
	memcpy(&packet->data[1], name, (u8)strlen(name));
	packet->data[(u8)strlen(name) + 1] = (u8)strlen(tld);
	memcpy(&packet->data[(u8)strlen(name) + 2], tld, (u8)strlen(tld));
	packet->data[(u8)strlen(name) + (u8)strlen(tld) + 2] = 0x00; // Name end
	packet->data[(u8)strlen(name) + (u8)strlen(tld) + 4] = 0x01; // A
	packet->data[(u8)strlen(name) + (u8)strlen(tld) + 6] = 0x01; // IN
}

static u32 dns_handle_packet(struct dns_packet *packet)
{
	u16 flags = htons(packet->flags);
	u8 reply_code = flags & 0xf;
	if (reply_code != DNS_NOERROR) {
		printf("DNS error: %d\n", reply_code);
		return 0;
	}

	u8 *start = &packet->data[1] + strlen((char *)&packet->data[1]);
	printf("TTL of %s: %ds\n", &packet->data[1], (u32)start[14]);
	u8 *ip = &start[17];
	printf("IP: %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
	return ip(ip[0], ip[1], ip[2], ip[3]);
}

u32 dns_request(const char *name, const char *tld)
{
	struct socket *socket = net_open(S_UDP);
	if (!socket || !net_connect(socket, dns_ip_addr, 53))
		return 0;

	u32 length = sizeof(struct dns_packet) + strlen(name) + strlen(tld) + 7; // TODO: 7 :)
	struct dns_packet *packet = malloc(length);
	memset(packet, 0, length);
	dns_make_packet(packet, name, tld);
	net_send(socket, packet, length);
	free(packet);

	u8 buf[1024] = { 0 };
	int l = net_receive(socket, buf, 1024);
	net_close(socket);
	if (l > 0)
		return dns_handle_packet((void *)buf);
	else
		return 0;
}

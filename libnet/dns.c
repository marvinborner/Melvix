// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>
#include <mem.h>
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

void dns_request(const char *name, const char *tld)
{
	struct socket *socket = net_open(S_UDP);
	if (socket)
		socket->src_port = 50053;
	if (!socket || !net_connect(socket, dns_ip_addr, 53))
		return;

	u32 length = sizeof(struct dns_packet) + strlen(name) + strlen(tld) + 7; // TODO: 7 :)
	struct dns_packet *packet = malloc(length);
	memset(packet, 0, length);
	dns_make_packet(packet, name, tld);
	net_send(socket, packet, length);
	free(packet);
}

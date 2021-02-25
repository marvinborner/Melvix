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

static u32 part_count(const char *name)
{
	u32 cnt = 0;
	for (u32 i = 0; i < strlen(name); i++) {
		if (name[i] == '.')
			cnt++;
	}
	return cnt + 1;
}

static u32 part_len(const char *name, u32 index)
{
	const char *data = name;

	u32 cnt = 0;
	for (u32 i = 0; i < strlen(name); i++) {
		if (cnt == index) {
			data += i;
			break;
		}

		if (name[i] == '.')
			cnt++;
	}

	for (cnt = 0; cnt < strlen(data); cnt++) {
		if (data[cnt] == '.' || data[cnt] == '\0')
			break;
	}

	return cnt;
}

static void dns_make_packet(struct dns_packet *packet, const char *name)
{
	packet->qid = htons(rand());
	packet->flags = htons(0x0100); // Standard query
	packet->questions = htons(1);
	packet->answers = htons(0);
	packet->authorities = htons(0);
	packet->additional = htons(0);

	u8 *data = packet->data;
	u32 cnt = 0;
	for (u32 i = 0; i < part_count(name) * 2; i += 2) {
		data[cnt] = part_len(name, i / 2);
		memcpy(&data[cnt + 1], &name[cnt], data[cnt]);
		cnt += data[cnt] + 1;
	}

	packet->data[cnt + 0] = 0x00; // Name end
	packet->data[cnt + 2] = 0x01; // A
	packet->data[cnt + 4] = 0x01; // IN
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

u32 dns_request(const char *name)
{
	struct socket *socket = net_open(S_UDP);
	if (!socket || !net_connect(socket, dns_ip_addr, 53, NET_TIMEOUT) || part_count(name) == 1)
		return 0;

	u32 length = sizeof(struct dns_packet) + strlen(name) + part_count(name) + 4;
	struct dns_packet *packet = malloc(length);
	memset(packet, 0, length);
	dns_make_packet(packet, name);
	net_send(socket, packet, length);
	free(packet);

	u8 buf[1024] = { 0 };
	int l = net_receive(socket, buf, 1024, NET_TIMEOUT);
	net_close(socket);
	if (l > 0)
		return dns_handle_packet((void *)buf);
	else
		return 0;
}

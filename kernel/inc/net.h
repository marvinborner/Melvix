// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef NET_H
#define NET_H

#include <def.h>

#define htonl(l)                                                                                   \
	((((l)&0xFF) << 24) | (((l)&0xFF00) << 8) | (((l)&0xFF0000) >> 8) |                        \
	 (((l)&0xFF000000) >> 24))
#define htons(s) ((((s)&0xFF) << 8) | (((s)&0xFF00) >> 8))
#define ntohl(l) htonl((l))
#define ntohs(s) htons((s))

#define ETHERNET_TYPE_IP4 0x0800
#define ETHERNET_TYPE_IP6 0x86dd
#define ETHERNET_TYPE_ARP 0x0806

#define IP_PROT_ICMP 0x01
#define IP_PROT_TCP 0x06
#define IP_PROT_UDP 0x11

struct ethernet_packet {
	u8 dst_mac_addr[6];
	u8 src_mac_addr[6];
	u16 type;
	u8 data[];
} __attribute__((packed));

struct ip_packet {
	u8 version : 4;
	u8 ihl : 4;
	u8 dscp_ecn;
	u16 length;
	u16 id;
	u16 flags_fragment;
	u8 ttl;
	u8 protocol;
	u16 checksum;
	u32 src;
	u32 dst;
	u8 data[];
} __attribute__((packed));

void ethernet_handle_packet(struct ethernet_packet *packet, int len);
void net_install();

#endif

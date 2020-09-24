// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef NET_H
#define NET_H

#include <def.h>

#define ETHERNET_TYPE_IP4 0x0800
#define ETHERNET_TYPE_IP6 0x86dd
#define ETHERNET_TYPE_ARP 0x0806

struct ethernet_packet {
	u8 dst_mac_addr[6];
	u8 src_mac_addr[6];
	u16 type;
	u8 data[];
} __attribute__((packed));

struct ip_packet {
	u8 version_ihl;
	u8 dscp_ecn;
	u16 length;
	u16 id;
	u8 flags_fragment;
	u8 ttl;
	u8 protocol;
	u16 checksum;
	u8 src_ip[4];
	u8 dst_ip[4];
	u8 data[];
} __attribute__((packed));

void ethernet_handle_packet(struct ethernet_packet *packet, int len);
void net_install();

#endif

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

#define ARP_REQUEST 1
#define ARP_REPLY 2

#define DHCP_REQUEST 1
#define DHCP_REPLY 2
#define DHCP_TRANSACTION_IDENTIFIER 0x55555555

#define HARDWARE_TYPE_ETHERNET 0x01

// Protocol structs

struct ethernet_packet {
	u8 dst[6];
	u8 src[6];
	u16 type;
	u8 data[];
} __attribute__((packed));

struct arp_packet {
	u16 hardware_type;
	u16 protocol;
	u8 hardware_addr_len;
	u8 protocol_addr_len;
	u16 opcode;
	u8 src_mac[6];
	u32 src_protocol_addr;
	u8 dst_mac[6];
	u32 dst_protocol_addr;
} __attribute__((packed));

struct ip_packet {
	u8 version_ihl;
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

struct dhcp_packet {
	u8 op;
	u8 hardware_type;
	u8 mac_len;
	u8 hops;
	u32 xid;
	u16 seconds;
	u16 flags;
	u32 client_ip;
	u32 your_ip;
	u32 server_ip;
	u32 gateway_ip;
	u8 client_mac[6];
	u8 reserved[10];
	u8 server_name[64];
	u8 file[128];
	u8 options[64];
} __attribute__((packed));

struct udp_packet {
	u16 src_port;
	u16 dst_port;
	u16 length;
	u16 checksum;
	u8 data[];
} __attribute__((packed));

struct icmp_packet {
	u8 type;
	u8 version;
	u16 checksum;
	u16 identifier;
	u16 sequence;
} __attribute__((packed));

// Other structs

struct arp_table_entry {
	u32 ip_addr;
	u64 mac_addr;
};

void ethernet_handle_packet(struct ethernet_packet *packet, int len);
void net_install();

#endif

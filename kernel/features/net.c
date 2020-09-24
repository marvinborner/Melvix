// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>
#include <net.h>
#include <pci.h>
#include <print.h>
#include <rtl8139.h>

u8 ntohb(u8 byte, int num_bits);
u16 ntohs(u16 netshort);
u32 ntohl(u32 netlong);

void ip_handle_packet(struct ip_packet *packet, int len)
{
	/* printf("V%d\n", packet->version_ihl); */
	u32 test = (u32)packet->src_ip[0];
	u8 *ip = (u8 *)ntohl(test);
	printf("IP %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
}

void ethernet_handle_packet(struct ethernet_packet *packet, int len)
{
	void *data = packet + sizeof(*packet);
	int data_len = len - sizeof(*packet);
	if (ntohs(packet->type) == ETHERNET_TYPE_ARP) {
		print("ARP PACKET\n");
	} else if (ntohs(packet->type) == ETHERNET_TYPE_IP4) {
		print("IP4 PACKET\n");
		ip_handle_packet(data, data_len);
	} else if (ntohs(packet->type) == ETHERNET_TYPE_IP6) {
		print("IP6 PACKET\n");
		ip_handle_packet(data, data_len);
	} else {
		printf("UNKNOWN PACKET %x\n", ntohs(packet->type));
	}
}

void net_install()
{
	rtl8139_install();
}

/**
 * Utilities
 */

u16 flip_short(u16 short_int)
{
	u32 first_byte = *((u8 *)(&short_int));
	u32 second_byte = *((u8 *)(&short_int) + 1);
	return (first_byte << 8) | (second_byte);
}

u32 flip_long(u32 long_int)
{
	u32 first_byte = *((u8 *)(&long_int));
	u32 second_byte = *((u8 *)(&long_int) + 1);
	u32 third_byte = *((u8 *)(&long_int) + 2);
	u32 fourth_byte = *((u8 *)(&long_int) + 3);
	return (first_byte << 24) | (second_byte << 16) | (third_byte << 8) | (fourth_byte);
}

u8 flip_byte(u8 byte, int num_bits)
{
	u8 t = byte << (8 - num_bits);
	return t | (byte >> num_bits);
}

u8 htonb(u8 byte, int num_bits)
{
	return flip_byte(byte, num_bits);
}

u8 ntohb(u8 byte, int num_bits)
{
	return flip_byte(byte, 8 - num_bits);
}

u16 htons(u16 hostshort)
{
	return flip_short(hostshort);
}

u32 htonl(u32 hostlong)
{
	return flip_long(hostlong);
}

u16 ntohs(u16 netshort)
{
	return flip_short(netshort);
}

u32 ntohl(u32 netlong)
{
	return flip_long(netlong);
}

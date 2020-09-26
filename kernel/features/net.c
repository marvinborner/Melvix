// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>
#include <mem.h>
#include <net.h>
#include <pci.h>
#include <print.h>
#include <rtl8139.h>

/**
 * Checksums
 */

u16 ip_calculate_checksum(struct ip_packet *packet)
{
	u32 sum = 0;
	u16 *s = (u16 *)packet;

	for (int i = 0; i < 10; ++i)
		sum += ntohs(s[i]);

	if (sum > 0xFFFF)
		sum = (sum >> 16) + (sum & 0xFFFF);

	return sum;
}

/**
 * Requests
 */

int ethernet_send_packet(u8 *dst, u8 *data, int len, int prot)
{
	struct ethernet_packet *packet = malloc(sizeof(*packet) + len);
	memcpy(packet->src, rtl8139_get_mac(), 6);
	memcpy(packet->dst, dst, 6);
	memcpy(packet->data, data, len);
	packet->type = htons(prot);
	rtl8139_send_packet(packet, sizeof(*packet) + len);
	free(packet);
	return len;
}

void ip_send_packet(u32 dst, void *data, int len, int prot)
{
	struct ip_packet *packet = malloc(sizeof(*packet) + len);
	memset(packet, 0, sizeof(*packet));
	packet->version_ihl = ((0x4 << 4) | (0x5 << 0));
	packet->length = sizeof(*packet) + len;
	packet->id = 0; // TODO: IP fragmentation
	packet->ttl = 64;
	packet->protocol = prot;
	packet->src = htonl(0x0a00022a);
	packet->dst = dst;
	/* void *packet_data = (u32 *)packet + 0x5 * 4; */
	memcpy(packet->data, data, len);
	packet->length = htons(sizeof(*packet) + len);
	packet->checksum = htons(ip_calculate_checksum(packet));
	// TODO: arp destination lookup
	ethernet_send_packet((u8 *)0x424242424242, (u8 *)packet, htons(packet->length),
			     ETHERNET_TYPE_IP4);
	free(packet);
}

/**
 * Responses
 */

void icmp_handle_packet(u32 dst, int len)
{
	struct icmp_packet *packet = malloc(sizeof(*packet) + len);
	memset(packet, 0, sizeof(*packet));
	packet->type = 0; // Ping reponse
	packet->version = 0;
	packet->checksum = 0;
	ip_send_packet(dst, packet, sizeof(*packet) + len, IP_PROT_ICMP);
	free(packet);
}

void ip_handle_packet(struct ip_packet *packet, int len)
{
	switch (packet->protocol) {
	case IP_PROT_ICMP:
		print("ICMP Packet!\n");
		icmp_handle_packet(packet->src, len);
		break;
	case IP_PROT_TCP:
		print("TCP Packet!\n");
		break;
	case IP_PROT_UDP:
		print("UDP Packet!\n");
		break;
	default:
		printf("Unknown IP protocol %d\n", packet->protocol);
	}
}

void ethernet_handle_packet(struct ethernet_packet *packet, int len)
{
	void *data = packet->data;
	int data_len = len - sizeof(*packet);
	if (ntohs(packet->type) == ETHERNET_TYPE_ARP) {
		print("ARP PACKET\n");
	} else if (ntohs(packet->type) == ETHERNET_TYPE_IP4) {
		print("IP4 PACKET\n");
		ip_handle_packet(data, data_len);
	} else if (ntohs(packet->type) == ETHERNET_TYPE_IP6) {
		print("IP6 PACKET\n");
		/* ip_handle_packet(data, data_len); */
	} else {
		printf("UNKNOWN PACKET %x\n", ntohs(packet->type));
	}
}

/**
 * Install
 */

void net_install()
{
	rtl8139_install();
}

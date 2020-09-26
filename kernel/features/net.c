// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>
#include <mem.h>
#include <net.h>
#include <pci.h>
#include <print.h>
#include <rtl8139.h>

int ethernet_send_packet(u8 *dst_mac_addr, u8 *data, int len, int prot)
{
	struct ethernet_packet *packet = malloc(sizeof(*packet) + len);
	void *packet_data = (u32 *)packet + sizeof(*packet);
	memcpy(packet->src_mac_addr, rtl8139_get_mac(), 6);
	memcpy(packet->dst_mac_addr, dst_mac_addr, 6);
	memcpy(packet_data, data, len);
	packet->type = htons(prot);
	rtl8139_send_packet(packet, sizeof(*packet) + len);
	free(packet);
	return len;
}

u16 ip_calculate_checksum(struct ip_packet *packet)
{
	int array_size = sizeof(*packet) / 2;
	u16 *array = (u16 *)packet;
	u32 sum = 0;
	for (int i = 0; i < array_size; i++) {
		u32 first_byte = *((u8 *)(&array[i]));
		u32 second_byte = *((u8 *)(&array[i]) + 1);
		sum += (first_byte << 8) | (second_byte);
	}
	u32 carry = sum >> 16;
	sum = sum & 0x0000ffff;
	sum = sum + carry;
	u16 ret = ~sum;
	return ret;
}

void ip_send_packet(u32 dst, void *data, int len, int prot)
{
	struct ip_packet *packet = malloc(sizeof(*packet) + len);
	memset(packet, 0, sizeof(*packet));
	packet->version = 4;
	packet->ihl = 5; // 5 * 4 = 20B
	packet->length = sizeof(*packet) + len;
	packet->id = 0; // TODO: IP fragmentation
	packet->ttl = 64;
	packet->protocol = prot;
	packet->src = 0;
	packet->dst = dst;
	void *packet_data = (u32 *)packet + packet->ihl * 4;
	memcpy(packet_data, data, len);
	packet->length = htons(sizeof(*packet) + len);
	packet->checksum = htons(ip_calculate_checksum(packet));
	// TODO: arp destination lookup
	ethernet_send_packet((u8 *)0x424242424242, (u8 *)packet, htons(packet->length),
			     ETHERNET_TYPE_IP4);
}

void ip_handle_packet(struct ip_packet *packet, int len)
{
	switch (packet->protocol) {
	case IP_PROT_ICMP:
		print("ICMP Packet!\n");
		ip_send_packet(packet->src, packet->data, len, IP_PROT_ICMP);
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

void net_install()
{
	rtl8139_install();
}

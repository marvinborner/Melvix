// MIT License, Copyright (c) 2020 Marvin Borner

#include <cpu.h>
#include <def.h>
#include <mem.h>
#include <net.h>
#include <pci.h>
#include <print.h>
#include <rtl8139.h>
#include <str.h>

static u32 ip_addr = 0x0f02000a;
static u32 gateway_addr = 0x0202000a;
static u8 gateway_mac[6] = { 0 };

/**
 * Helper functions
 */

u16 ip_calculate_checksum(struct ip_packet *packet)
{
	int array_size = sizeof(*packet) / 2;
	u16 *array = (u16 *)packet;
	u32 sum = 0;
	for (int i = 0; i < array_size; i++) {
		sum += htons(array[i]);
	}
	u32 carry = sum >> 16;
	sum = sum & 0x0000ffff;
	sum = sum + carry;
	u16 ret = ~sum;
	return ret;
}

u16 tcp_calculate_checksum(struct tcp_packet *packet, struct tcp_pseudo_header *header, void *data,
			   u32 len)
{
	u32 sum = 0;
	u16 *s = (u16 *)header;

	// TODO: Checksums for options?
	for (int i = 0; i < 6; ++i) {
		sum += ntohs(s[i]);
		if (sum > 0xffff) {
			sum = (sum >> 16) + (sum & 0xffff);
		}
	}

	s = (u16 *)packet;
	for (int i = 0; i < 10; ++i) {
		sum += ntohs(s[i]);
		if (sum > 0xffff) {
			sum = (sum >> 16) + (sum & 0xffff);
		}
	}

	u16 d_words = len / 2;

	s = (u16 *)data;
	for (unsigned int i = 0; i < d_words; ++i) {
		sum += ntohs(s[i]);
		if (sum > 0xffff) {
			sum = (sum >> 16) + (sum & 0xffff);
		}
	}

	if (d_words * 2 != len) {
		u8 *t = (u8 *)data;
		u8 tmp[2];
		tmp[0] = t[d_words * sizeof(u16)];
		tmp[1] = 0;

		u16 *f = (u16 *)tmp;

		sum += ntohs(f[0]);
		if (sum > 0xffff) {
			sum = (sum >> 16) + (sum & 0xffff);
		}
	}

	return ~(sum & 0xffff) & 0xffff;
}

u16 icmp_calculate_checksum(struct icmp_packet *packet)
{
	u32 sum = 0;
	u16 *s = (u16 *)packet;

	for (int i = 0; i < 5; i++)
		sum += s[i];

	if (sum > 0xffff)
		sum = (sum >> 16) + (sum & 0xffff);

	return ~sum;
}

void *dhcp_get_options(struct dhcp_packet *packet, u8 type)
{
	u8 *options = packet->options + 4;
	u8 curr_type = 0;
	while ((curr_type = *options) != 0xff) {
		u8 len = *(options + 1);
		if (curr_type == type) {
			void *ret = malloc(len);
			memcpy(ret, options + 2, len);
			return ret;
		}
		options += (2 + len);
	}
	return NULL;
}

/**
 * Requests
 */

void ethernet_send_packet(u8 *dst, u8 *data, int len, int prot)
{
	print("Ethernet send packet\n");
	struct ethernet_packet *packet = malloc(sizeof(*packet) + len);
	memcpy(packet->src, rtl8139_get_mac(), 6);
	memcpy(packet->dst, dst, 6);
	memcpy(packet->data, data, len);
	packet->type = htons(prot);
	rtl8139_send_packet(packet, sizeof(*packet) + len);
	free(packet);
}

static u8 broadcast_mac[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
void arp_send_packet(u8 *dst_mac, u32 dst_protocol_addr, u8 opcode)
{
	print("ARP send packet\n");
	struct arp_packet *packet = malloc(sizeof(*packet));

	memcpy(packet->src_mac, rtl8139_get_mac(), 6);
	packet->src_protocol_addr = ip_addr;
	memcpy(packet->dst_mac, dst_mac, 6);
	packet->dst_protocol_addr = dst_protocol_addr;
	packet->opcode = htons(opcode);
	packet->hardware_addr_len = 6;
	packet->protocol_addr_len = 4;
	packet->hardware_type = htons(HARDWARE_TYPE_ETHERNET);
	packet->protocol = htons(ETHERNET_TYPE_IP4);

	ethernet_send_packet(broadcast_mac, (u8 *)packet, sizeof(*packet), ETHERNET_TYPE_ARP);
	free(packet);
}

int arp_lookup(u8 *ret_hardware_addr, u32 ip_addr);
void ip_send_packet(u32 dst, void *data, int len, int prot)
{
	print("IP send packet\n");
	struct ip_packet *packet = malloc(sizeof(*packet) + len);
	memset(packet, 0, sizeof(*packet));
	packet->version_ihl = ((0x4 << 4) | (0x5 << 0));
	packet->length = sizeof(*packet) + len;
	packet->id = htons(1); // TODO: IP fragmentation
	packet->ttl = 64;
	packet->protocol = prot;
	packet->src = ip_addr;
	packet->dst = dst;
	packet->length = htons(sizeof(*packet) + len);
	packet->checksum = htons(ip_calculate_checksum(packet));

	if (data)
		memcpy(packet->data, data, len);

	u8 dst_mac[6];

	int arp_sent = 3;
	u8 zero_hardware_addr[] = { 0, 0, 0, 0, 0, 0 };
	sti();
	while (!arp_lookup(dst_mac, dst)) {
		if (arp_sent) {
			arp_sent--;
			arp_send_packet(zero_hardware_addr, dst, ARP_REQUEST);
		} else {
			break;
		}
	}
	cli();
	printf("Destination: %x:%x:%x:%x:%x:%x\n", dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3],
	       dst_mac[4], dst_mac[5]);
	ethernet_send_packet(dst_mac, (u8 *)packet, htons(packet->length), ETHERNET_TYPE_IP4);

	free(packet);
}

void udp_send_packet(u32 dst, u16 src_port, u16 dst_port, void *data, int len)
{
	print("UDP send packet\n");
	int length = sizeof(struct udp_packet) + len;
	struct udp_packet *packet = malloc(length);
	memset(packet, 0, sizeof(*packet));
	packet->src_port = htons(src_port);
	packet->dst_port = htons(dst_port);
	packet->length = htons(length);
	packet->checksum = 0; // Optional

	if (data)
		memcpy(packet->data, data, len);

	ip_send_packet(dst, packet, length, IP_PROT_UDP);
	free(packet);
}

u32 seq_no, ack_no = 0; // TODO: Per socket
void tcp_send_packet(u32 dst, u16 src_port, u16 dst_port, u16 flags, void *data, int len)
{
	print("TCP send packet\n");
	int length = sizeof(struct tcp_packet) + len;
	struct tcp_packet *packet = malloc(length);
	memset(packet, 0, sizeof(*packet));
	packet->src_port = htons(src_port);
	packet->dst_port = htons(dst_port);
	packet->seq_number = htonl(seq_no);
	packet->ack_number = flags & TCP_FLAG_ACK ? htonl(ack_no) : 0;
	packet->flags = htons(0x5000 ^ (flags & 0xff));
	packet->window_size = htons(1548 - 54);
	packet->urgent = 0;
	packet->checksum = 0; // Later

	if ((flags & 0xff) == TCP_FLAG_SYN)
		seq_no++;
	else
		seq_no += len;

	if (data)
		memcpy(packet->data, data, len);

	struct tcp_pseudo_header checksum_hd = {
		.src = ip_addr,
		.dst = dst,
		.zeros = 0,
		.protocol = 6,
		.tcp_len = htons(length),
	};
	u16 checksum = tcp_calculate_checksum(packet, &checksum_hd, data,
					      length - (htons(packet->flags) >> 12) * 4);
	packet->checksum = htons(checksum);

	ip_send_packet(dst, packet, length, IP_PROT_TCP);
	free(packet);
}

void dhcp_make_packet(struct dhcp_packet *packet, u8 msg_type);
void dhcp_request()
{
	u32 dst = 0xffffffff;
	struct dhcp_packet *packet = malloc(sizeof(*packet));
	memset(packet, 0, sizeof(*packet));
	dhcp_make_packet(packet, 3);
	udp_send_packet(dst, 68, 67, packet, sizeof(*packet));
	free(packet);
}

/**
 * Responses
 */

void icmp_handle_packet(struct icmp_packet *request_packet, u32 dst)
{
	struct icmp_packet *packet = malloc(sizeof(*packet));
	memset(packet, 0, sizeof(*packet));
	packet->type = 0; // Ping reponse
	packet->version = 0;
	packet->checksum = 0;
	packet->identifier = request_packet->identifier;
	packet->sequence = request_packet->sequence;
	packet->checksum = icmp_calculate_checksum(packet);
	ip_send_packet(dst, packet, sizeof(*packet), IP_PROT_ICMP);
	free(packet);
}

void dhcp_handle_packet(struct dhcp_packet *packet)
{
	print("DHCP!\n");
	if (packet->op == DHCP_REPLY && htonl(packet->xid) == DHCP_TRANSACTION_IDENTIFIER) {
		u8 *type = dhcp_get_options(packet, 53);
		if (*type == 2) { // Offer
			print("DHCP offer\n");
			dhcp_request();
		} else if (*type == 5) { // ACK
			ip_addr = packet->your_ip;
			printf("ACK! New IP: %x\n", ip_addr);
		}
		free(type);
	}
}

void tcp_handle_packet(struct tcp_packet *packet, u32 dst, int len)
{
	printf("TCP Port: %d\n", ntohs(packet->dst_port));
	int data_length = len - (htons(packet->flags) >> 12) * 4;
	u16 flags = ntohs(packet->flags);
	printf("%b\n", flags);

	if (seq_no != ntohl(packet->ack_number)) {
		printf("Dropping packet seq_no: %d\n", seq_no);
		return;
	}

	if ((htons(packet->flags) & TCP_FLAG_SYN) && (htons(packet->flags) & TCP_FLAG_ACK)) {
		ack_no = ntohl(packet->seq_number) + data_length + 1;
		tcp_send_packet(dst, packet->dst_port, packet->src_port, TCP_FLAG_ACK, NULL, 0);
	} else if (htons(packet->flags) & TCP_FLAG_RES) {
		print("Socket reset!\n");
		return;
	} else if (data_length == 0) {
		if (htons(packet->flags) & TCP_FLAG_FIN) {
			print("Finished, closing socket\n");
			ack_no = ntohl(packet->seq_number) + data_length + 1;
			tcp_send_packet(dst, packet->dst_port, packet->src_port,
					TCP_FLAG_ACK | TCP_FLAG_FIN, NULL, 0);
		}
		return;
	}

	ack_no = ntohl(packet->seq_number) + data_length;
	if ((htons(packet->flags) & TCP_FLAG_SYN) && (htons(packet->flags) & TCP_FLAG_ACK) &&
	    data_length == 0)
		ack_no++;
	ack_no = ntohl(packet->seq_number) + data_length;

	tcp_send_packet(dst, packet->dst_port, packet->src_port, TCP_FLAG_ACK, NULL, 0);

	// TODO: Look at the spec again
	if (htons(packet->flags) & TCP_FLAG_FIN) {
		print("Finished, closing socket\n");
		ack_no = ntohl(packet->seq_number) + data_length + 1;
		tcp_send_packet(dst, packet->dst_port, packet->src_port,
				TCP_FLAG_ACK | TCP_FLAG_FIN, NULL, 0);
	}
}

void udp_handle_packet(struct udp_packet *packet)
{
	printf("UDP Port: %d\n", ntohs(packet->dst_port));
	void *data_ptr = (u8 *)packet + sizeof(*packet);

	if (ntohs(packet->dst_port) == 68)
		dhcp_handle_packet(data_ptr);
}

void ip_handle_packet(struct ip_packet *packet, int len)
{
	(void)len;
	switch (packet->protocol) {
	case IP_PROT_ICMP:
		print("ICMP Packet!\n");
		icmp_handle_packet((struct icmp_packet *)packet->data, packet->src);
		break;
	case IP_PROT_TCP:
		print("TCP Packet!\n");
		tcp_handle_packet((struct tcp_packet *)packet->data, packet->src, len);
		break;
	case IP_PROT_UDP:
		print("UDP Packet!\n");
		udp_handle_packet((struct udp_packet *)packet->data);
		break;
	default:
		break;
	}
}

static struct arp_table_entry arp_table[512];
static int arp_table_size;
static int arp_table_curr;
void arp_handle_packet(struct arp_packet *packet, int len)
{
	(void)len;
	u8 dst_mac[6];
	memcpy(dst_mac, packet->src_mac, 6);
	u32 dst_protocol_addr = packet->src_protocol_addr;
	if (ntohs(packet->opcode) == ARP_REQUEST) {
		print("Got ARP request\n");
		if (packet->dst_protocol_addr == ip_addr) {
			print("Returning ARP request\n");
			arp_send_packet(dst_mac, dst_protocol_addr, ARP_REPLY);
		}
	} else if (ntohs(packet->opcode) == ARP_REPLY) {
		print("Got ARP reply\n");
	} else {
		printf("Got unknown ARP, opcode = %d\n", packet->opcode);
	}

	// Store
	arp_table[arp_table_curr].ip_addr = dst_protocol_addr;
	memcpy(&arp_table[arp_table_curr].mac_addr, dst_mac, 6);
	if (arp_table_size < 512)
		arp_table_size++;
	if (arp_table_curr >= 512)
		arp_table_curr = 0;
}

void ethernet_handle_packet(struct ethernet_packet *packet, int len)
{
	void *data = packet->data;
	int data_len = len - sizeof(*packet);
	if (ntohs(packet->type) == ETHERNET_TYPE_ARP) {
		print("ARP PACKET\n");
		arp_handle_packet(data, data_len);
	} else if (ntohs(packet->type) == ETHERNET_TYPE_IP4) {
		print("IP4 PACKET\n");
		ip_handle_packet(data, data_len);
	} else if (ntohs(packet->type) == ETHERNET_TYPE_IP6) {
		print("IP6 PACKET\n");
		/* ip_handle_packet(data, data_len); */
	} else {
		/* printf("Unknown packet %x\n", ntohs(packet->type)); */
	}
}

/**
 * DHCP
 */

void dhcp_make_packet(struct dhcp_packet *packet, u8 msg_type)
{
	packet->op = DHCP_REQUEST;
	packet->hardware_type = HARDWARE_TYPE_ETHERNET;
	packet->mac_len = 6;
	packet->hops = 0;
	packet->xid = htonl(DHCP_TRANSACTION_IDENTIFIER);
	packet->flags = htons(0x0001);
	memcpy(packet->client_mac, rtl8139_get_mac(), 6);

	// Magic Cookie
	u8 *options = packet->options;
	*((u32 *)(options)) = htonl(0x63825363);
	options += 4;

	// First option, message type = DHCP_DISCOVER/DHCP_REQUEST
	*(options++) = 53;
	*(options++) = 1;
	*(options++) = msg_type;

	// End
	*(options++) = 0xff;
}

void dhcp_discover()
{
	print("DHCP discover\n");
	u32 dst_ip = 0xffffffff;
	struct dhcp_packet *packet = malloc(sizeof(*packet));
	memset(packet, 0, sizeof(*packet));
	dhcp_make_packet(packet, 1);
	udp_send_packet(dst_ip, 68, 67, packet, sizeof(*packet));
	free(packet);
}

/**
 * ARP
 */

void arp_lookup_add(u8 *ret_hardware_addr, u32 ip_addr)
{
	arp_table[arp_table_curr].ip_addr = ip_addr;
	memcpy(&arp_table[arp_table_curr].mac_addr, ret_hardware_addr, 6);
	if (arp_table_size < 512)
		arp_table_size++;
	if (arp_table_curr >= 512)
		arp_table_curr = 0;
}

int arp_lookup(u8 *ret_hardware_addr, u32 ip_addr)
{
	for (int i = 0; i < 512; i++) {
		if (arp_table[i].ip_addr == ip_addr) {
			memcpy(ret_hardware_addr, &arp_table[i].mac_addr, 6);
			return 1;
		}
	}
	return 0;
}

/**
 * Install
 */

void net_install()
{
	if (rtl8139_install()) {
		sti();
		arp_send_packet(broadcast_mac, gateway_addr, ARP_REQUEST);
		print("Waiting for gateway answer...\n");
		while (!arp_lookup(gateway_mac, gateway_addr))
			hlt(); // TODO: Add ARP timeout
		print("Found gateway!\n");

		arp_lookup_add(gateway_mac, 0xffffffff);
		dhcp_discover();
	}
}

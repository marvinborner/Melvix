// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <cpu.h>
#include <def.h>
#include <mem.h>
#include <net.h>
#include <pci.h>
#include <print.h>
#include <rtl8139.h>
#include <str.h>

static u32 current_ip_addr = 0x0f02000a;
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
		sum += (u32)htons(array[i]);
	}
	u32 carry = sum >> 16;
	sum = sum & 0x0000ffff;
	sum = sum + carry;
	u16 ret = (u16)~sum;
	return ret;
}

u16 tcp_calculate_checksum(struct tcp_packet *packet, struct tcp_pseudo_header *header, void *data,
			   u32 len)
{
	u32 sum = 0;
	u16 *s = (u16 *)header;

	// TODO: Checksums for options?
	for (int i = 0; i < 6; i++) {
		sum += (u32)ntohs(s[i]);
		if (sum > 0xffff) {
			sum = (sum >> 16) + (sum & 0xffff);
		}
	}

	s = (u16 *)packet;
	for (int i = 0; i < 10; i++) {
		sum += (u32)ntohs(s[i]);
		if (sum > 0xffff) {
			sum = (sum >> 16) + (sum & 0xffff);
		}
	}

	u16 d_words = (u16)(len / 2);

	s = (u16 *)data;
	for (u32 i = 0; i < d_words; ++i) {
		sum += (u32)ntohs(s[i]);
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

		sum += (u32)ntohs(f[0]);
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

	return (u16)~sum;
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
	struct ethernet_packet *packet = malloc(sizeof(*packet) + (u32)len);
	memcpy(packet->src, rtl8139_get_mac(), 6);
	memcpy(packet->dst, dst, 6);
	memcpy(packet->data, data, (u32)len);
	packet->type = (u16)htons(prot);
	rtl8139_send_packet(packet, sizeof(*packet) + (u32)len);
	free(packet);
}

static u8 broadcast_mac[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
void arp_send_packet(u8 *dst_mac, u32 dst_protocol_addr, u8 opcode)
{
	print("ARP send packet\n");
	struct arp_packet *packet = malloc(sizeof(*packet));

	memcpy(packet->src_mac, rtl8139_get_mac(), 6);
	packet->src_protocol_addr = current_ip_addr;
	memcpy(packet->dst_mac, dst_mac, 6);
	packet->dst_protocol_addr = dst_protocol_addr;
	packet->opcode = (u16)htons(opcode);
	packet->hardware_addr_len = 6;
	packet->protocol_addr_len = 4;
	packet->hardware_type = htons(HARDWARE_TYPE_ETHERNET);
	packet->protocol = htons(ETHERNET_TYPE_IP4);

	ethernet_send_packet(broadcast_mac, (u8 *)packet, sizeof(*packet), ETHERNET_TYPE_ARP);
	free(packet);
}

int arp_lookup(u8 *ret_hardware_addr, u32 ip_addr);
void ip_send_packet(u32 dst, void *data, int len, u8 prot)
{
	print("IP send packet\n");
	struct ip_packet *packet = malloc(sizeof(*packet) + (u32)len);
	memset(packet, 0, sizeof(*packet));
	packet->version_ihl = ((0x4 << 4) | (0x5 << 0));
	packet->length = (u16)sizeof(*packet) + (u16)len;
	packet->id = htons(1); // TODO: IP fragmentation
	packet->ttl = 64;
	packet->protocol = prot;
	packet->src = current_ip_addr;
	packet->dst = dst;
	packet->length = (u16)htons(sizeof(*packet) + (u32)len);
	packet->checksum = (u16)htons(ip_calculate_checksum(packet));

	if (data)
		memcpy(packet->data, data, (u32)len);

	int arp_sent = 3;
	u8 zero_hardware_addr[] = { 0, 0, 0, 0, 0, 0 };
	u8 dst_mac[6];
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
	u32 length = sizeof(struct udp_packet) + (u32)len;
	struct udp_packet *packet = malloc(length);
	memset(packet, 0, sizeof(*packet));
	packet->src_port = (u16)htons(src_port);
	packet->dst_port = (u16)htons(dst_port);
	packet->length = (u16)htons(length);
	packet->checksum = 0; // Optional

	if (data)
		memcpy(packet->data, data, (u32)len);

	ip_send_packet(dst, packet, (int)length, IP_PROT_UDP);
	free(packet);
}

static u32 seq_no, ack_no, tcp_state = 0; // TODO: Per socket
void tcp_send_packet(u32 dst, u16 src_port, u16 dst_port, u16 flags, void *data, int len)
{
	print("TCP send packet\n");
	u32 length = sizeof(struct tcp_packet) + (u32)len;
	struct tcp_packet *packet = malloc(length);
	memset(packet, 0, sizeof(*packet));
	packet->src_port = (u16)htons(src_port);
	packet->dst_port = (u16)htons(dst_port);
	packet->seq_number = htonl(seq_no);
	packet->ack_number = flags & TCP_FLAG_ACK ? htonl(ack_no) : 0;
	packet->flags = (u16)htons(0x5000 ^ (flags & 0xff));
	packet->window_size = htons(1024);
	packet->urgent = 0;
	packet->checksum = 0; // Later

	if (data)
		memcpy(packet->data, data, (u32)len);

	struct tcp_pseudo_header checksum_hd = {
		.src = current_ip_addr,
		.dst = dst,
		.zeros = 0,
		.protocol = 6,
		.tcp_len = (u16)htons(length),
	};
	u16 checksum = tcp_calculate_checksum(packet, &checksum_hd, data,
					      length - ((u32)htons(packet->flags) >> 12) * 4);
	packet->checksum = (u16)htons(checksum);

	ip_send_packet(dst, packet, (int)length, IP_PROT_TCP);
	free(packet);
}

void dhcp_make_packet(struct dhcp_packet *packet, u8 msg_type);
void dhcp_request(void)
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
			current_ip_addr = packet->your_ip;
			printf("ACK! New IP: %x\n", current_ip_addr);
		}
		free(type);
	}
}

// enum tcp_state { TCP_LISTEN, TCP_SYN_SENT, TCP_SYN_RECIEVED, TCP_ESTABLISHED, TCP_FIN_WAIT_1, TCP_FIN_WAIT_2, TCP_CLOSE_WAIT, TCP_CLOSING, TCP_LAST_ACK, TCP_TIME_WAIT, TCP_CLOSED };
// TODO: Fix TCP retransmission (dropped packages; probably race condition)
//#define test_http "HTTP/1.1 200"
#define test_http "HTTP/1.2 200\nContent-Length: 14\nConnection: close\n\n<h1>Hallo</h1>"
void tcp_handle_packet(struct tcp_packet *packet, u32 dst, int len)
{
	printf("TCP Port: %d\n", ntohs(packet->dst_port));
	u32 data_length = (u32)len - (htons(packet->flags) >> 12) * 4;
	u16 flags = (u16)ntohs(packet->flags);

	u32 recv_ack = ntohl(packet->ack_number);
	u32 recv_seq = ntohl(packet->seq_number);

	if (tcp_state == 0 && (flags & 0xff) == TCP_FLAG_SYN) {
		ack_no = recv_seq + 1;
		seq_no = 1000;
		tcp_send_packet(dst, ntohs(packet->dst_port), ntohs(packet->src_port),
				TCP_FLAG_SYN | TCP_FLAG_ACK, NULL, 0);
		tcp_state++;
		return;
	} else if (tcp_state == 1 && (flags & 0xff) == TCP_FLAG_ACK) {
		/* assert(recv_ack == seq_no + 1); */

		tcp_state++;
		return;
	} else if (tcp_state == 2 && (flags & 0xff) == (TCP_FLAG_ACK | TCP_FLAG_PSH)) {
		/* assert(recv_ack == seq_no + 1); */

		/* for (u32 i = 0; i < data_length; ++i) { */
		/* 	if (packet->data[i]) */
		/* 		printf("%c", packet->data[i]); */
		/* } */

		ack_no += data_length;
		seq_no++;

		tcp_send_packet(dst, ntohs(packet->dst_port), ntohs(packet->src_port), TCP_FLAG_ACK,
				NULL, 0);
		tcp_send_packet(dst, ntohs(packet->dst_port), ntohs(packet->src_port),
				TCP_FLAG_PSH | TCP_FLAG_ACK, strdup(test_http), strlen(test_http));
		return;
	} else if (tcp_state == 2 && (flags & 0xff) == (TCP_FLAG_ACK)) {
		ack_no = recv_seq + 1;
		seq_no = recv_ack;

		tcp_state = 3;
		return;
	} else if (tcp_state == 3 && (flags & 0xff) == (TCP_FLAG_ACK | TCP_FLAG_FIN)) {
		ack_no = recv_seq + 1;
		seq_no = recv_ack;

		tcp_send_packet(dst, ntohs(packet->dst_port), ntohs(packet->src_port),
				TCP_FLAG_FIN | TCP_FLAG_ACK, NULL, 0);

		tcp_state = 0;
		return;
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
		tcp_handle_packet((struct tcp_packet *)packet->data, packet->src,
				  ntohs(packet->length) - sizeof(*packet));
		break;
	case IP_PROT_UDP:
		print("UDP Packet!\n");
		udp_handle_packet((struct udp_packet *)packet->data);
		break;
	default:
		break;
	}
}

static struct arp_table_entry arp_table[512] = { 0 };
static int arp_table_size = 0;
void arp_handle_packet(struct arp_packet *packet, int len)
{
	(void)len;
	u8 dst_mac[6];
	memcpy(dst_mac, packet->src_mac, 6);
	u32 dst_protocol_addr = packet->src_protocol_addr;
	if (ntohs(packet->opcode) == ARP_REQUEST) {
		print("Got ARP request\n");
		if (packet->dst_protocol_addr == current_ip_addr) {
			print("Returning ARP request\n");
			arp_send_packet(dst_mac, dst_protocol_addr, ARP_REPLY);
		}
		return;
	} else if (ntohs(packet->opcode) == ARP_REPLY) {
		print("Got ARP reply\n");
	} else {
		printf("Got unknown ARP, opcode = %d\n", packet->opcode);
		return;
	}

	// Store
	arp_table[arp_table_size].ip_addr = dst_protocol_addr;
	memcpy(&arp_table[arp_table_size].mac_addr, dst_mac, 6);
	if (arp_table_size < 512)
		arp_table_size++;
	else
		arp_table_size = 0;
}

void ethernet_handle_packet(struct ethernet_packet *packet, int len)
{
	void *data = packet->data;
	int data_len = len - (int)sizeof(*packet);
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

void dhcp_discover(void)
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
	arp_table[arp_table_size].ip_addr = ip_addr;
	memcpy(&arp_table[arp_table_size].mac_addr, ret_hardware_addr, 6);
	if (arp_table_size < 512)
		arp_table_size++;
	else
		arp_table_size = 0;
}

int arp_lookup(u8 *ret_hardware_addr, u32 ip_addr)
{
	for (int i = 0; i < arp_table_size; i++) {
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

void net_install(void)
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

// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <cpu.h>
#include <def.h>
#include <list.h>
#include <mem.h>
#include <net.h>
#include <pci.h>
#include <print.h>
#include <random.h>
#include <rtl8139.h>
#include <str.h>
#include <timer.h>

static u32 current_ip_addr = 0;
static u32 gateway_addr = 0;
static u32 subnet_mask = 0;
static u8 gateway_mac[6] = { 0 };

static struct list *tcp_sockets = NULL;
static struct list *udp_sockets = NULL;

/**
 * Socket functions
 */

struct socket *socket_get(struct list *list, u32 port)
{
	if (!list || !list->head || !port)
		return NULL;

	struct node *iterator = list->head;
	while (iterator != NULL && iterator->data != NULL) {
		if (((struct socket *)iterator->data)->src_port == port)
			return iterator->data;
		iterator = iterator->next;
	}

	return NULL;
}

struct socket *socket_new(struct list *list)
{
	struct socket *socket = malloc(sizeof(*socket));
	memset(socket, 0, sizeof(*socket));
	if (!list_add(list, socket))
		return NULL;

	return socket;
}

/**
 * Helper functions
 */

static u16 next_port(void)
{
	static u16 port = 49152;
	return port++;
}

static int same_net(u32 ip_addr)
{
	if (gateway_addr && ip_addr > 1)
		return (ip_addr & subnet_mask) == (gateway_addr & subnet_mask);
	else
		return 0;
}

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
	packet->src_protocol_addr = htonl(current_ip_addr);
	memcpy(packet->dst_mac, dst_mac, 6);
	packet->dst_protocol_addr = htonl(dst_protocol_addr);
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
	packet->src = htonl(current_ip_addr);
	packet->dst = htonl(dst);
	packet->length = (u16)htons(sizeof(*packet) + (u32)len);
	packet->checksum = (u16)htons(ip_calculate_checksum(packet));

	if (data)
		memcpy(packet->data, data, (u32)len);

	int arp_sent = 3;
	u8 zero_hardware_addr[] = { 0, 0, 0, 0, 0, 0 };
	u8 dst_mac[6];
	sti();
	if (same_net(dst))
		while (!arp_lookup(dst_mac, dst)) {
			if (arp_sent) {
				arp_sent--;
				arp_send_packet(zero_hardware_addr, dst, ARP_REQUEST);
			} else {
				break;
			}
		}
	else
		memcpy(dst_mac, gateway_mac, 6);

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

//void tcp_send_packet(u32 dst, u16 src_port, u16 dst_port, u16 flags, void *data, int len)
void tcp_send_packet(struct socket *socket, u16 flags, void *data, int len)
{
	print("TCP send packet\n");
	u32 length = sizeof(struct tcp_packet) + (u32)len;
	struct tcp_packet *packet = malloc(length);
	memset(packet, 0, sizeof(*packet));
	packet->src_port = (u16)htons(socket->src_port);
	packet->dst_port = (u16)htons(socket->dst_port);
	packet->seq_number = htonl(socket->prot.tcp.seq_no);
	packet->ack_number = flags & TCP_FLAG_ACK ? htonl(socket->prot.tcp.ack_no) : 0;
	packet->flags = (u16)htons(0x5000 ^ (flags & 0xff));
	packet->window_size = htons((2 << 15) - 1); // TODO: Support TCP windows
	packet->urgent = 0;
	packet->checksum = 0; // Later

	if (data)
		memcpy(packet->data, data, (u32)len);

	struct tcp_pseudo_header checksum_hd = {
		.src = htonl(current_ip_addr),
		.dst = htonl(socket->ip_addr),
		.zeros = 0,
		.protocol = 6,
		.tcp_len = (u16)htons(length),
	};
	u16 checksum = tcp_calculate_checksum(packet, &checksum_hd, data,
					      length - ((u32)htons(packet->flags) >> 12) * 4);
	packet->checksum = (u16)htons(checksum);

	ip_send_packet(socket->ip_addr, packet, (int)length, IP_PROT_TCP);
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
			current_ip_addr = htonl(packet->your_ip);

			memcpy(&subnet_mask, dhcp_get_options(packet, 1), 4);
			memcpy(&gateway_addr, dhcp_get_options(packet, 3), 4);
			memcpy(gateway_mac, dhcp_get_options(packet, 3), 4);
			subnet_mask = htonl(subnet_mask);
			gateway_addr = htonl(gateway_addr);

			u8 zero_hardware_addr[] = { 0, 0, 0, 0, 0, 0 };
			printf("New IP: %x\n", current_ip_addr);
			printf("Gateway: %x\n", gateway_addr);
			if (same_net(current_ip_addr))
				arp_send_packet(zero_hardware_addr, gateway_addr, ARP_REQUEST);
			/* sti(); */
			/* while (!arp_lookup(gateway_mac, gateway_addr)) */
			/* 	hlt(); */
		}
		free(type);
	}
}

// enum tcp_state { TCP_LISTEN, TCP_SYN_SENT, TCP_SYN_RECIEVED, TCP_ESTABLISHED, TCP_FIN_WAIT_1, TCP_FIN_WAIT_2, TCP_CLOSE_WAIT, TCP_CLOSING, TCP_LAST_ACK, TCP_TIME_WAIT, TCP_CLOSED };
#define http_res "HTTP/1.1 200\r\nContent-Length: 14\r\nConnection: close\r\n\r\n<h1>Hallo</h1>"
#define http_req "GET / HTTP/1.1\r\nHost: marvinborner.de\r\n\r\n"
void tcp_handle_packet(struct tcp_packet *packet, u32 dst, int len)
{
	printf("TCP Port: %d\n", ntohs(packet->dst_port));

	struct socket *socket = NULL;
	if (!(socket = socket_get(tcp_sockets, ntohs(packet->dst_port)))) {
		print("Port isn't mapped!\n");
		return;
	}
	struct tcp_socket *tcp = &socket->prot.tcp;

	u32 data_length = (u32)len - (ntohs(packet->flags) >> 12) * 4;
	u16 flags = (u16)ntohs(packet->flags);

	u32 recv_ack = ntohl(packet->ack_number);
	u32 recv_seq = ntohl(packet->seq_number);

	// TODO: Verify checksum first, then send ACK

	// Serve
	if (tcp->state == 0 && (flags & 0xff) == TCP_FLAG_SYN) {
		socket->ip_addr = dst;
		socket->dst_port = ntohs(packet->src_port);
		tcp->ack_no = recv_seq + 1;
		tcp->seq_no = 1000;
		tcp_send_packet(socket, TCP_FLAG_SYN | TCP_FLAG_ACK, NULL, 0);
		tcp->state++;
		return;
	} else if (tcp->state == 1 && (flags & 0xff) == TCP_FLAG_ACK) {
		/* assert(recv_ack == seq_no + 1); */

		tcp->state++;
		return;
	} else if (tcp->state == 2 && (flags & 0xff) == (TCP_FLAG_ACK | TCP_FLAG_PSH)) {
		/* assert(recv_ack == seq_no + 1); */

		/* for (u32 i = 0; i < data_length; ++i) { */
		/* 	if (packet->data[i]) */
		/* 		printf("%c", packet->data[i]); */
		/* } */

		tcp->ack_no += data_length;
		tcp->seq_no++;

		tcp_send_packet(socket, TCP_FLAG_ACK, NULL, 0);
		/* tcp_send_packet(socket, TCP_FLAG_PSH | TCP_FLAG_ACK, strdup(http_res), */
		/* 		strlen(http_res)); */

		tcp->state++;
		return;
	} else if (tcp->state == 3 && (flags & 0xff) == TCP_FLAG_ACK) {
		tcp->ack_no = recv_seq + 1;
		tcp->seq_no = recv_ack;

		/* tcp->state++; */
		return;
	} else if (tcp->state == 4 && (flags & 0xff) == (TCP_FLAG_ACK | TCP_FLAG_FIN)) {
		tcp->ack_no = recv_seq + 1;
		tcp->seq_no = recv_ack;

		tcp_send_packet(socket, TCP_FLAG_FIN | TCP_FLAG_ACK, NULL, 0);

		tcp->state = 0;
		return;
	}

	// Receive
	if (tcp->state == 0 && (flags & 0xff) == (TCP_FLAG_ACK | TCP_FLAG_SYN)) {
		tcp->ack_no = recv_seq + 1;
		tcp->seq_no = recv_ack;

		tcp_send_packet(socket, TCP_FLAG_ACK, NULL, 0);
		/* tcp_send_packet(socket, TCP_FLAG_PSH | TCP_FLAG_ACK, strdup(http_req), */
		/* 		strlen(http_req)); */

		/* tcp->ack_no += strlen(http_req); */

		tcp->state = 5; // TODO: TCP enum state machine
		return;
	} else if (tcp->state == 5 && (flags & 0xff) == TCP_FLAG_ACK) {
		tcp->ack_no = recv_seq;
		tcp->seq_no = recv_ack;

		tcp->state++;
		return;
	} else if (tcp->state == 6 && (flags & 0xff) == (TCP_FLAG_ACK | TCP_FLAG_PSH)) {
		for (u32 i = 0; i < data_length; ++i) {
			if (packet->data[i])
				printf("%c", packet->data[i]);
		}

		tcp->ack_no += data_length;
		tcp->seq_no = recv_ack;

		tcp_send_packet(socket, TCP_FLAG_ACK, NULL, 0);
		tcp_send_packet(socket, TCP_FLAG_FIN | TCP_FLAG_ACK, NULL, 0);

		tcp->state++;
		return;
	} else if (tcp->state == 7 && (flags & 0xff) == (TCP_FLAG_ACK | TCP_FLAG_FIN)) {
		tcp->ack_no = recv_seq + 1;
		tcp->seq_no = recv_ack;

		tcp_send_packet(socket, TCP_FLAG_ACK, NULL, 0);

		tcp->state = 0;
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
	u32 dst_protocol_addr = htonl(packet->src_protocol_addr);
	if (ntohs(packet->opcode) == ARP_REQUEST) {
		print("Got ARP request\n");
		if (htonl(packet->dst_protocol_addr) == current_ip_addr) {
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

	if (dst_protocol_addr == gateway_addr)
		memcpy(gateway_mac, dst_mac, 6);
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
 * Wrappers
 */

struct socket *net_open(enum socket_type type)
{
	struct list *sockets = type == S_TCP ? tcp_sockets : udp_sockets; // TODO
	struct socket *socket = socket_new(sockets);
	socket->type = type;
	return socket;
}

int net_connect(struct socket *socket, u32 ip_addr, u16 dst_port)
{
	socket->ip_addr = ip_addr;
	socket->dst_port = dst_port;
	if (!socket->src_port)
		socket->src_port = next_port();

	if (socket->type == S_TCP) {
		srand(timer_get());
		socket->prot.tcp.seq_no = rand();
		socket->prot.tcp.ack_no = 0;
		socket->prot.tcp.state = 0;
		tcp_send_packet(socket, TCP_FLAG_SYN, NULL, 0);
		struct tcp_socket *tcp = &socket->prot.tcp;
		sti();
		u32 time = timer_get();
		while (tcp->state != 3 && tcp->state != 5 && timer_get() - time < 1000)
			;
		cli();
		if (tcp->state != 3 && tcp->state != 5)
			return 0;
	} else {
		return 0;
	}

	return 1;
}

void net_send(struct socket *socket, void *data, u32 len)
{
	// TODO: UDP socket support
	tcp_send_packet(socket, TCP_FLAG_PSH | TCP_FLAG_ACK, data, len);
	socket->prot.tcp.ack_no += len;
}

/**
 * Install
 */

void net_install(void)
{
	if (!rtl8139_install())
		return;

	sti();

	arp_lookup_add(broadcast_mac, 0xffffffff);
	dhcp_discover();

	u32 time = timer_get();
	while (!arp_lookup(gateway_mac, gateway_addr) && timer_get() - time < 1000)
		timer_wait(10);

	if (timer_get() - time >= 1000) {
		printf("Gateway ARP timeout at address %x\n", gateway_addr);
		loop();
		return;
	}

	tcp_sockets = list_new();
	udp_sockets = list_new();

	// Request
	struct socket *socket = net_open(S_TCP);
	if (net_connect(socket, ip(91, 89, 253, 227), 80))
		net_send(socket, strdup(http_req), strlen(http_req));
	else
		print("Couldn't connect!\n");

	// Server // TODO: Serve using sockets
	/* struct socket *socket2 = net_open(S_TCP); */
	/* socket2->src_port = 8000; */
	/* while (socket2->prot.tcp.state != 3) */
	/* 	; */
	/* while (socket2->prot.tcp.state == 3) */
	/* 	net_send(socket2, strdup(http_res), strlen(http_res)); */
}

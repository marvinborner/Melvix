// MIT License, Copyright (c) 2020 Marvin Borner
// Uses parts of the ToAruOS Project, released under the terms of the NCSA
// Copyright (C) 2011-2018 K. Lange

#include <cpu.h>
#include <def.h>
#include <interrupts.h>
#include <mem.h>
#include <net.h>
#include <pci.h>
#include <print.h>

static u8 mac[6];
static u8 *rx_buffer;
u32 current_packet_ptr;

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

void ethernet_handle_packet(struct ethernet_packet *packet, int len)
{
	/* void *data = packet + sizeof(*packet); */
	printf("", len);
	if (ntohs(packet->type) == ETHERNET_TYPE_ARP)
		print("ARP PACKET\n");
	else if (ntohs(packet->type) == ETHERNET_TYPE_IP4)
		print("IP4 PACKET\n");
	else if (ntohs(packet->type) == ETHERNET_TYPE_IP6)
		print("IP6 PACKET\n");
	else
		printf("UNKNOWN PACKET %x\n", ntohs(packet->type));
}

/**
 * RTL8139 specific things
 */

static int rtl_irq = 0;
static u32 rtl_device_pci = 0;
static u32 rtl_iobase = 0;

void rtl_receive_packet()
{
	u16 *t = (u16 *)(rx_buffer + current_packet_ptr);
	u16 length = *(t + 1);
	t += 2;

	void *packet = malloc(length);
	memcpy(packet, t, length);
	ethernet_handle_packet(packet, length);

	current_packet_ptr = (current_packet_ptr + length + 4 + 3) & (~3);

	if (current_packet_ptr > RX_BUF_SIZE)
		current_packet_ptr -= RX_BUF_SIZE;

	outw(rtl_iobase + RTL_PORT_RXPTR, current_packet_ptr - 0x10);
}

void rtl8139_find(u32 device, u16 vendor_id, u16 device_id, void *extra)
{
	if ((vendor_id == 0x10ec) && (device_id == 0x8139)) {
		*((u32 *)extra) = device;
	}
}

void rtl8139_irq_handler()
{
	print("RTL INT!\n");
	u16 status = inw(rtl_iobase + RTL_PORT_ISR);
	if (!status)
		return;

	if (status & RTL_TOK) {
		printf("Sent packet\n");
	} else if (status & RTL_ROK) {
		rtl_receive_packet();
	}

	outw(rtl_iobase + RTL_PORT_ISR, 0x5);
}

void rtl8139_init()
{
	if (!rtl_device_pci)
		return;

	u16 command_reg = (u16)pci_read_field(rtl_device_pci, PCI_COMMAND, 4);
	if ((command_reg & (1 << 2)) == 0) {
		command_reg |= (1 << 2);
		pci_write_field(rtl_device_pci, PCI_COMMAND, command_reg);
	}

	rtl_irq = pci_get_interrupt(rtl_device_pci);
	irq_install_handler(rtl_irq, rtl8139_irq_handler);

	u32 rtl_bar0 = pci_read_field(rtl_device_pci, PCI_BAR0, 4);
	/* u32 rtl_bar1 = pci_read_field(rtl_device_pci, PCI_BAR1, 4); */
	rtl_iobase = 0;

	if (rtl_bar0 & 0x00000001)
		rtl_iobase = rtl_bar0 & (~0x3);

	// Get mac address
	for (int i = 0; i < 6; ++i)
		mac[i] = inb(rtl_iobase + RTL_PORT_MAC + i);
	printf("Mac address: %x:%x:%x:%x:%x:%x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	// Activate
	outb(rtl_iobase + RTL_PORT_CONFIG, 0x0);

	// Reset
	outb((u16)(rtl_iobase + RTL_PORT_CMD), 0x10);
	while ((inb(rtl_iobase + RTL_PORT_CMD) & 0x10) != 0)
		;

	// Set receive buffer
	rx_buffer = (u8 *)malloc(RX_BUF_SIZE);
	memset(rx_buffer, 0, RX_BUF_SIZE);
	outl(rtl_iobase + RTL_PORT_RBSTART, (u32)rx_buffer);

	// Set TOK and ROK
	outw(rtl_iobase + RTL_PORT_IMR, 0x0005);

	// AB+AM+APM+AAP except WRAP
	outl(rtl_iobase + RTL_PORT_RCR, 0xf | (1 << 7));

	// Enable receive and transmit
	outb(rtl_iobase + RTL_PORT_CMD, 0x08 | 0x04);
}

void net_install()
{
	pci_scan(&rtl8139_find, -1, &rtl_device_pci);

	if (rtl_device_pci) {
		print("Found rtl8139 card\n");
		rtl8139_init();
	}
}

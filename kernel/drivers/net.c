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

void ethernet_handle_packet(struct ethernet_packet *packet)
{
	/* void *data = packet + sizeof(*packet); */
	if (ntohs(packet->type) == ETHERNET_TYPE_ARP)
		print("ARP PACKET\n");
	else if (ntohs(packet->type) == ETHERNET_TYPE_IP)
		print("IP PACKET\n");
	else
		printf("UNKNOWN PACKET %d\n", ntohs(packet->type));
}

static int rtl_irq = 0;
static u8 mac[6];
static u8 *last_packet = NULL;
static u8 *rtl_rx_buffer;
static u32 rtl_device_pci = 0;
static u32 rtl_iobase = 0;
static u32 cur_rx = 0;

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
	outw(rtl_iobase + RTL_PORT_ISR, status);

	if (status & 0x01 || status & 0x02) {
		while ((inb(rtl_iobase + RTL_PORT_CMD) & 0x01) == 0) {
			print("RECEIVE\n");

			int offset = cur_rx % 0x2000;

			u32 *buf_start = (u32 *)((u32)rtl_rx_buffer + offset);
			u32 rx_status = buf_start[0];
			u32 rx_size = rx_status >> 16;

			if (rx_status & (0x0020 | 0x0010 | 0x0004 | 0x0002)) {
				print("RX Error\n");
			} else {
				u8 *buf_8 = (u8 *)&(buf_start[1]);

				last_packet = malloc(rx_size);

				u32 packet_end = (u32)buf_8 + rx_size;
				if (packet_end > (u32)rtl_rx_buffer + 0x2000) {
					u32 s = ((u32)rtl_rx_buffer + 0x2000) - (u32)buf_8;
					memcpy(last_packet, buf_8, s);
					memcpy((void *)((u32)last_packet + s), rtl_rx_buffer,
					       rx_size - s);
				} else {
					memcpy(last_packet, buf_8, rx_size);
				}

				ethernet_handle_packet((struct ethernet_packet *)last_packet);
			}

			cur_rx = (cur_rx + rx_size + 4 + 3) & ~3;
			outw(rtl_iobase + RTL_PORT_RXPTR, cur_rx - 16);
		}
	}
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
		rtl_iobase = rtl_bar0 & 0xFFFFFFFC;

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
	rtl_rx_buffer = (u8 *)malloc(0x3000);
	memset(rtl_rx_buffer, 0, 0x3000);
	outl(rtl_iobase + RTL_PORT_RBSTART, (u32)rtl_rx_buffer);

	// Enable IRQs
	outw(rtl_iobase + RTL_PORT_IMR, 0x8000 | /* PCI error */
						0x4000 | /* PCS timeout */
						0x40 | /* Rx FIFO over */
						0x20 | /* Rx underrun */
						0x10 | /* Rx overflow */
						0x08 | /* Tx error */
						0x04 | /* Tx okay */
						0x02 | /* Rx error */
						0x01 /* Rx okay */
	);

	// Configure transmit
	outl(rtl_iobase + RTL_PORT_TCR, 0);

	// Configure receive
	outl(rtl_iobase + RTL_PORT_RCR, (0) | /* 8K receive */
						0x08 | /* broadcast */
						0x01 /* all physical */
	);

	// Enable receive and transmit
	outb(rtl_iobase + RTL_PORT_CMD, 0x08 | 0x04);

	// Reset rx statistics
	outl(rtl_iobase + RTL_PORT_RXMISS, 0);
}

void net_install()
{
	pci_scan(&rtl8139_find, -1, &rtl_device_pci);

	if (rtl_device_pci) {
		print("Found rtl8139 card\n");
		rtl8139_init();
	}
}

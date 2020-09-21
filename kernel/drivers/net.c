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
	u16 status = inw(rtl_iobase + 0x3E);
	if (!status)
		return;
	outw(rtl_iobase + 0x3E, status);

	if (status & 0x01 || status & 0x02) {
		while ((inb(rtl_iobase + 0x37) & 0x01) == 0) {
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

				/* rtl_enqueue(last_packet); */
			}

			cur_rx = (cur_rx + rx_size + 4 + 3) & ~3;
			outw(rtl_iobase + 0x3A, cur_rx - 16);
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
	rtl_iobase = 0;

	if (rtl_bar0 & 0x00000001)
		rtl_iobase = rtl_bar0 & 0xFFFFFFFC;

	// Get mac address
	for (int i = 0; i < 6; ++i)
		mac[i] = inb((u16)(rtl_iobase + 0x00 + i));
	printf("Mac address: %x:%x:%x:%x:%x:%x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	// Activate
	outb((u16)(rtl_iobase + 0x52), 0x0);

	// Reset
	outb((u16)(rtl_iobase + 0x37), 0x10);
	while ((inb((u16)(rtl_iobase + 0x37)) & 0x10) != 0)
		;

	// Set receive buffer
	rtl_rx_buffer = (u8 *)malloc(0x3000);
	memset(rtl_rx_buffer, 0x00, 0x3000);
	outl((u16)(rtl_iobase + 0x30), (u32)rtl_rx_buffer);

	// Enable ISR
	outw((u16)(rtl_iobase + 0x3C), 0x0005);

	// Accept packets
	outl((u16)(rtl_iobase + 0x44), 0xf | (1 << 7));

	// Enable receive and transmitter
	outb((u16)(rtl_iobase + 0x37), 0x0C);
}

void net_install()
{
	pci_scan(&rtl8139_find, -1, &rtl_device_pci);

	if (rtl_device_pci) {
		print("Found rtl8139 card\n");
		rtl8139_init();
	}
}

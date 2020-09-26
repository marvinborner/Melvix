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
#include <rtl8139.h>

static u8 mac[6];
static u8 *rx_buffer;
static u32 current_packet_ptr;

static u32 rtl_device_pci = 0;
static u32 rtl_iobase = 0;

u8 *rtl8139_get_mac()
{
	return mac;
}

void rtl8139_receive_packet()
{
	printf("%x\n", current_packet_ptr);
	u16 *t = (u16 *)(rx_buffer + current_packet_ptr);
	u16 length = *(t + 1);
	t += 2;

	void *packet = malloc(length);
	memcpy(packet, t, length);
	ethernet_handle_packet(packet, length);

	current_packet_ptr = (current_packet_ptr + length + 4 + 3) & (~3);

	if (current_packet_ptr >= RX_BUF_SIZE)
		current_packet_ptr -= RX_BUF_SIZE;

	outw(rtl_iobase + RTL_PORT_RXPTR, current_packet_ptr - 0x10);
}

static u8 tsad_array[4] = { 0x20, 0x24, 0x28, 0x2C };
static u8 tsd_array[4] = { 0x10, 0x14, 0x18, 0x1C };
static u8 tx_current = 0;
void rtl8139_send_packet(void *data, u32 len)
{
	printf("Sending packet %d\n", len);
	outl(rtl_iobase + tsad_array[tx_current], (u32)data);
	outl(rtl_iobase + tsd_array[tx_current++], len);
	if (tx_current > 3)
		tx_current = 0;
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

	if (status & RTL_TOK) {
		print("Sent packet\n");
	} else if (status & RTL_ROK) {
		rtl8139_receive_packet();
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
	rx_buffer = (u8 *)malloc(RX_BUF_SIZE + 1516);
	memset(rx_buffer, 0, RX_BUF_SIZE + 1516);
	outl(rtl_iobase + RTL_PORT_RBSTART, (u32)rx_buffer);

	// Set TOK and ROK
	outw(rtl_iobase + RTL_PORT_IMR, 0x0005);

	// AB+AM+APM+AAP except WRAP
	outl(rtl_iobase + RTL_PORT_RCR, 0xf | (1 << 7));

	// Enable receive and transmit
	outb(rtl_iobase + RTL_PORT_CMD, 0x08 | 0x04);

	// Install interrupt handler
	u32 rtl_irq = pci_get_interrupt(rtl_device_pci);
	irq_install_handler(rtl_irq, rtl8139_irq_handler);
}

void rtl8139_install()
{
	pci_scan(&rtl8139_find, -1, &rtl_device_pci);

	if (rtl_device_pci) {
		print("Found rtl8139 card\n");
		rtl8139_init();
	}
}

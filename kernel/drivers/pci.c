// MIT License, Copyright (c) 2020 Marvin Borner
// Uses parts of the ToAruOS Project, released under the terms of the NCSA
// Copyright (C) 2011-2018 K. Lange

#include <cpu.h>
#include <def.h>
#include <mem.h>
#include <pci.h>

CLEAR void pci_write_field(u32 device, int field, u32 value)
{
	outl(PCI_ADDRESS_PORT, pci_get_addr(device, field));
	outl(PCI_VALUE_PORT, value);
}

CLEAR u32 pci_read_field(u32 device, int field, int size)
{
	outl(PCI_ADDRESS_PORT, pci_get_addr(device, field));

	if (size == 4) {
		u32 t = inl(PCI_VALUE_PORT);
		return t;
	} else if (size == 2) {
		u16 t = inw((u16)(PCI_VALUE_PORT + (field & 2)));
		return t;
	} else if (size == 1) {
		u8 t = inb((u16)(PCI_VALUE_PORT + (field & 3)));
		return t;
	}
	return 0xFFFF;
}

CLEAR u16 pci_find_type(u32 device)
{
	return (u16)((pci_read_field(device, PCI_CLASS, 1) << 8) |
		     pci_read_field(device, PCI_SUBCLASS, 1));
}

CLEAR void pci_scan_hit(pci_func_t f, u32 dev, void *extra)
{
	int dev_vend = (int)pci_read_field(dev, PCI_VENDOR_ID, 2);
	int dev_dvid = (int)pci_read_field(dev, PCI_DEVICE_ID, 2);

	f(dev, (u16)dev_vend, (u16)dev_dvid, extra);
}

CLEAR void pci_scan_func(pci_func_t f, int type, int bus, int slot, int func, void *extra)
{
	u32 dev = pci_box_device(bus, slot, func);
	if (type == -1 || type == pci_find_type(dev))
		pci_scan_hit(f, dev, extra);
	if (pci_find_type(dev) == PCI_TYPE_BRIDGE)
		pci_scan_bus(f, type, (int)pci_read_field(dev, PCI_SECONDARY_BUS, 1), extra);
}

CLEAR void pci_scan_slot(pci_func_t f, int type, int bus, int slot, void *extra)
{
	u32 dev = pci_box_device(bus, slot, 0);
	if (pci_read_field(dev, PCI_VENDOR_ID, 2) == PCI_NONE)
		return;
	pci_scan_func(f, type, bus, slot, 0, extra);
	if (!pci_read_field(dev, PCI_HEADER_TYPE, 1))
		return;
	for (int func = 1; func < 8; func++) {
		dev = pci_box_device(bus, slot, func);
		if (pci_read_field(dev, PCI_VENDOR_ID, 2) != PCI_NONE)
			pci_scan_func(f, type, bus, slot, func, extra);
	}
}

CLEAR void pci_scan_bus(pci_func_t f, int type, int bus, void *extra)
{
	for (int slot = 0; slot < 32; ++slot)
		pci_scan_slot(f, type, bus, slot, extra);
}

CLEAR void pci_scan(pci_func_t f, int type, void *extra)
{
	if ((pci_read_field(0, PCI_HEADER_TYPE, 1) & 0x80) == 0) {
		pci_scan_bus(f, type, 0, extra);
		return;
	}

	for (int func = 0; func < 8; ++func) {
		u32 dev = pci_box_device(0, 0, func);
		if (pci_read_field(dev, PCI_VENDOR_ID, 2) != PCI_NONE)
			pci_scan_bus(f, type, func, extra);
		else
			break;
	}
}

CLEAR static void find_isa_bridge(u32 device, u16 vendor_id, u16 device_id, void *extra)
{
	if (vendor_id == 0x8086 && (device_id == 0x7000 || device_id == 0x7110))
		*((u32 *)extra) = device;
}

PROTECTED static u32 pci_isa = 0;
PROTECTED static u8 pci_remaps[4] = { 0 };

// Remap
CLEAR void pci_install(void)
{
	pci_scan(&find_isa_bridge, -1, &pci_isa);
	if (pci_isa) {
		for (int i = 0; i < 4; ++i) {
			pci_remaps[i] = (u8)pci_read_field(pci_isa, 0x60 + i, 1);
			if (pci_remaps[i] == 0x80) {
				pci_remaps[i] = (u8)(10 + (i % 1));
			}
		}
		u32 out = 0;
		memcpy(&out, &pci_remaps, 4);
		pci_write_field(pci_isa, 0x60, out);
	}
}

CLEAR int pci_get_interrupt(u32 device)
{
	if (pci_isa) {
		u32 irq_pin = pci_read_field(device, 0x3D, 1);
		if (irq_pin == 0)
			return (int)pci_read_field(device, PCI_INTERRUPT_LINE, 1);
		int pirq = ((int)irq_pin + pci_extract_slot(device) - 2) % 4;
		int int_line = (int)pci_read_field(device, PCI_INTERRUPT_LINE, 1);

		if (pci_remaps[pirq] >= 0x80) {
			if (int_line == 0xFF) {
				int_line = 10;
				pci_write_field(device, PCI_INTERRUPT_LINE, (u32)int_line);
			}
			pci_remaps[pirq] = (u8)int_line;
			u32 out = 0;
			memcpy(&out, &pci_remaps, 4);
			pci_write_field(pci_isa, 0x60, out);
			return int_line;
		}
		pci_write_field(device, PCI_INTERRUPT_LINE, pci_remaps[pirq]);
		return pci_remaps[pirq];
	} else {
		return (int)pci_read_field(device, PCI_INTERRUPT_LINE, 1);
	}
}

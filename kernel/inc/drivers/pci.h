// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef PCI_H
#define PCI_H

#include <def.h>

#define PCI_VENDOR_ID 0x00 // 2
#define PCI_DEVICE_ID 0x02 // 2
#define PCI_COMMAND 0x04 // 2
#define PCI_STATUS 0x06 // 2
#define PCI_REVISION_ID 0x08 // 1

#define PCI_PROG_IF 0x09 // 1
#define PCI_SUBCLASS 0x0a // 1
#define PCI_CLASS 0x0b // 1
#define PCI_CACHE_LINE_SIZE 0x0c // 1
#define PCI_LATENCY_TIMER 0x0d // 1
#define PCI_HEADER_TYPE 0x0e // 1
#define PCI_BIST 0x0f // 1
#define PCI_BAR0 0x10 // 4
#define PCI_BAR1 0x14 // 4
#define PCI_BAR2 0x18 // 4
#define PCI_BAR3 0x1C // 4
#define PCI_BAR4 0x20 // 4
#define PCI_BAR5 0x24 // 4

#define PCI_INTERRUPT_LINE 0x3C // 1

#define PCI_SECONDARY_BUS 0x19 // 1

#define PCI_HEADER_TYPE_DEVICE 0
#define PCI_HEADER_TYPE_BRIDGE 1
#define PCI_HEADER_TYPE_CARDBUS 2

#define PCI_TYPE_BRIDGE 0x0604
#define PCI_TYPE_SATA 0x0106

#define PCI_ADDRESS_PORT 0xCF8
#define PCI_VALUE_PORT 0xCFC

#define PCI_NONE 0xFFFF

typedef void (*pci_func_t)(u32 device, u16 vendor_id, u16 device_id, void *extra);

struct pci_device_descriptor {
	u32 port_base;
	u32 interrupt;

	u8 bus;
	u8 slot;
	u8 func;

	u16 vendor_id;
	u16 device_id;

	u8 class_id;
	u8 subclass_id;
	u8 interface_id;

	u8 revision;
};

static inline u8 pci_extract_bus(u32 device)
{
	return (u8)((device >> 16));
}

static inline u8 pci_extract_slot(u32 device)
{
	return (u8)((device >> 8));
}

static inline u8 pci_extract_func(u32 device)
{
	return (u8)(device);
}

UNUSED_FUNC static inline u32 pci_get_addr(u32 device, int field)
{
	return 0x80000000 | (u32)(pci_extract_bus(device) << 16) |
	       (u32)(pci_extract_slot(device) << 11) | (u32)(pci_extract_func(device) << 8) |
	       ((field)&0xFC);
}

UNUSED_FUNC static inline u32 pci_box_device(int bus, int slot, int func)
{
	return (u32)((bus << 16) | (slot << 8) | func);
}

u32 pci_read_field(u32 device, int field, int size);
void pci_write_field(u32 device, int field, u32 value);
u16 pci_find_type(u32 dev);
void pci_scan_hit(pci_func_t f, u32 dev, void *extra) NONNULL;
void pci_scan_func(pci_func_t f, int type, int bus, int slot, int func, void *extra) NONNULL;
void pci_scan_slot(pci_func_t f, int type, int bus, int slot, void *extra) NONNULL;
void pci_scan_bus(pci_func_t f, int type, int bus, void *extra) NONNULL;
void pci_scan(pci_func_t f, int type, void *extra) NONNULL;
int pci_get_interrupt(u32 device);
void pci_install(void);

#endif

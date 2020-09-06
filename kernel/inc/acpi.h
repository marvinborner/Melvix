// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef ACPI_H
#define ACPI_H

#include <def.h>

#define RSDP_MAGIC "RSD PTR "
#define RSDT_MAGIC "RSDT"
#define MADT_MAGIC "APIC"
#define FADT_MAGIC "FACP"
#define HPET_MAGIC "HPET"

struct address_structure {
	u8 address_space_id;
	u8 register_bit_width;
	u8 register_bit_offset;
	u8 reserved;
	u32 phys; // Actually u64
};

struct sdt_header {
	char signature[4];
	u32 length;
	u8 revision;
	u8 checksum;
	char oem_id[6];
	char oem_table_id[8];
	u32 oem_revision;
	u32 creator_id;
	u32 creator_revision;
};

struct sdp_header {
	char signature[8];
	u8 checksum;
	char oem_id[6];
	u8 revision;
};

struct rsdt {
	struct sdt_header header;
	u32 sdt_pointer[];
};

struct madt {
	struct sdt_header header;
	u32 local_address;
	u32 flags;
};

struct fadt {
	struct sdt_header header;
	// TODO: FADT table (big!)
};

struct hpet {
	struct sdt_header header;
	u8 hardware_rev_id;
	u8 comparator_count : 5;
	u8 counter_size : 1;
	u8 reserved : 1;
	u8 legacy_replacement : 1;
	u16 pci_vendor_id;
	struct address_structure address;
	u8 hpet_number;
	u16 minimum_tick;
	u8 page_protection;
};

struct hpet_registers {
	struct {
		u8 revision;
		u8 comparator_count : 5;
		u8 counter_size : 1;
		u8 reserved : 1;
		u8 legacy_replacement : 1;
		u16 pci_vendor_id;
		u32 tick_period;
	} features;
	u64 reserved1;
	struct {
		u8 enable : 1;
		u8 legacy_replacement : 1;
		u64 reserved : 62;
	} config;
	u64 reserved2;
	struct {
		u32 status; // For timer #n
		u32 reserved;
	} int_status;
	u8 reserved3[200]; // Why?!
	u32 counter;
	u32 counter_high; // 0 due to 64 bit
	u64 reserved4;
	struct {
		u8 reserved1 : 1;
		u8 type : 1;
		u8 enable : 1;
		u8 periodic : 1;
		u8 periodic_support : 1;
		u8 size : 1; // 1 if 64 bit
		u8 set_accumulator : 1;
		u8 reserved2 : 1;
		u8 force_32 : 1; // For 64 bit
		u8 apic_routing : 5;
		u8 fsb : 1;
		u8 fsb_support : 1;
		u16 reserved3;
		u32 routing_capability;
	} timer;
};

struct rsdp {
	struct sdp_header header;
	struct rsdt *rsdt;
};

struct madt *madt;
struct fadt *fadt;
struct hpet *hpet;

void acpi_install();
void hpet_install();

#endif

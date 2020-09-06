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
	u64 address;
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

struct rsdp {
	struct sdp_header header;
	struct rsdt *rsdt;
};

struct madt *madt;
struct fadt *fadt;
struct hpet *hpet;

void acpi_install();

#endif

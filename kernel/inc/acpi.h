// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef ACPI_H
#define ACPI_H

#include <def.h>

#define RSDP_MAGIC "RSD PTR "
#define RSDT_MAGIC "RSDT"
#define MADT_MAGIC "APIC"
#define FADT_MAGIC "FACP"
#define HPET_MAGIC "HPET"

#define HPET_MAX_PERIOD 0x05F5E100

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

enum hpet_features { hpet_counter_size = 1 << 3, hpet_legacy_replacement_support = 1 << 5 };
enum hpet_config { hpet_enable = 1 << 0, hpet_legacy_replacement = 1 << 1 };
enum hpet_timer {
	hpet_type = 1 << 1,
	hpet_enable_timer = 1 << 2,
	hpet_periodic = 1 << 3,
	hpet_periodic_support = 1 << 4,
	hpet_size = 1 << 5, // 1 if 64 bit
	hpet_set_accumulator = 1 << 6,
	hpet_force_32 = 1 << 8, // For 64 bit
	hpet_apic_routing = 1 << 13,
	hpet_fsb = 1 << 14,
	hpet_fsb_support = 1 << 15,
	/* routing_capability = 1 << 63 */
};

struct hpet_registers {
	u32 features; // enum hpet_features
	u32 tick_period;
	u64 reserved1;
	u64 config; // enum hpet_config
	u64 reserved2;
	u32 int_status; // For timer #n
	u32 reserved3;
	u8 reserved4[200]; // Why?!
	u32 counter;
	u32 counter_high; // 0 due to 64 bit
	u64 reserved5;
	u64 timer0; // enum hpet_timer
	u64 timer_comparator0; // In femtoseconds
} __attribute__((packed));

struct rsdp {
	struct sdp_header header;
	struct rsdt *rsdt;
};

struct madt *madt;
struct fadt *fadt;
struct hpet *hpet;

void acpi_install();
void hpet_install(int frequency);

#endif

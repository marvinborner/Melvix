// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef ACPI_H
#define ACPI_H

#include <kernel.h>
#include <stdint.h>

#define RSDP_MAGIC "RSD PTR "
#define RSDT_MAGIC "RSDT"
#define MADT_MAGIC "APIC"
#define FADT_MAGIC "FACP"
#define HPET_MAGIC "HPET"

/**
 * General headers
 */

struct sdt_header {
	u8 signature[4];
	u32 length;
	u8 revision;
	u8 checksum;
	u8 oem_id[6];
	u8 oem_table_id[8];
	u32 oem_revision;
	u32 creator_id;
	u32 creator_revision;
};

struct sdp_header {
	u8 signature[8];
	u8 checksum;
	u8 oem_id[6];
	u8 revision;
};

struct generic_address {
	u8 address_space;
	u8 bit_width;
	u8 bit_offset;
	u8 access_size;
	u32 phys_low;
	u32 phys_high;
};

/**
 * RSDT
 */

struct rsdt {
	struct sdt_header header;
	u32 sdt_pointer[];
};

/**
 * RSDP - points to RSDT
 */

struct rsdp {
	struct sdp_header header;
	struct rsdt *rsdt;
};

/**
 * MADT
 */

struct madt_entry_header {
	u8 type;
	u8 length;
};

struct madt {
	struct sdt_header header;
	u32 local_address;
	u32 flags;
	struct madt_entry_header entry;
} PACKED;

/**
 * FADT
 */

struct fadt {
	struct sdt_header header;
	u32 firmware_ctrl;
	u32 dsdt;
	u8 reserved;
	u8 preferred_power_mgmt;
	u16 sci_interrupt;
	u32 smi_command_port;
	u8 acpi_enable;
	u8 acpi_disable;
	u8 s4_bios_req;
	u8 pstate_control;
	u32 pm1a_event_block;
	u32 pm1b_event_block;
	u32 pm1a_control_block;
	u32 pm1b_control_block;
	u32 pm2_control_block;
	u32 pm_timer_block;
	u32 gpe0_block;
	u32 gpe1_block;
	u8 pm1_event_length;
	u8 pm1_control_length;
	u8 pm2_control_length;
	u8 pm_timer_length;
	u8 gpe0_length;
	u8 gpe1_length;
	u8 gpe1_base;
	u8 c_state_control;
	u16 worst_c2_latency;
	u16 worst_c3_latency;
	u16 flush_size;
	u16 flush_stride;
	u8 duty_offset;
	u8 duty_width;
	u8 day_alarm;
	u8 month_alarm;
	u8 century;

	// Used since ACPI 2.0+
	u16 boot_architecture_flags;

	u8 reserved2;
	u32 flags;
	struct generic_address reset_reg;
	u8 reset_value;
	u8 reserved3[3];

	// Available on ACPI 2.0+
	u32 x_firmware_control_high;
	u32 x_firmware_control_low;
	u32 x_dsdt_high;
	u32 x_dsdt_low;

	struct generic_address x_pm1a_event_block;
	struct generic_address x_pm1b_event_block;
	struct generic_address x_pm1a_control_block;
	struct generic_address x_pm1b_control_block;
	struct generic_address x_pm2_control_block;
	struct generic_address x_pm_timer_block;
	struct generic_address x_gpe0_block;
	struct generic_address x_gpe1_block;
} PACKED;

/**
 * HPET
 */

struct hpet {
	struct sdt_header header;
	u8 hardware_rev_id;
	u8 comparator_count : 5;
	u8 counter_size : 1;
	u8 reserved : 1;
	u8 legacy_replacement : 1;
	u16 pci_vendor_id;
	struct generic_address address;
	u8 hpet_number;
	u16 minimum_tick;
	u8 page_protection;
} PACKED;

void acpi_probe(struct boot_information *info);

#endif

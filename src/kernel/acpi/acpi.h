#ifndef MELVIX_ACPI_H
#define MELVIX_ACPI_H

#include <multiboot.h>
#include <stdint.h>

/**
 * Initialize the ACP interface
 */
void acpi_old_init(struct multiboot_tag_old_acpi *tag);
void acpi_new_init(struct multiboot_tag_new_acpi *tag);

/**
 * Activate a ACPI based device reboot
 */
void reboot();

/**
 * Activate a ACPI based device shutdown/poweroff
 */
void acpi_poweroff();

struct address_structure {
	u8 address_space_id;
	u8 register_bit_width;
	u8 register_bit_offset;
	u8 reserved;
	u64 address;
};

// p. 138
struct fadt_flags {
	u8 WBINVD : 1;
	u8 WBINVD_flush : 1;
	u8 C1_support : 1;
	u8 C2_mp_support : 1;
	u8 power_button : 1; // 1 if not present
	u8 sleep_button : 1; // 1 if not present
	u8 rtc_fix_reg : 1;
	u8 rtc_wakes_S4 : 1;
	u8 timer_32 : 1;
	u8 dock_support : 1;
	u8 reset_support : 1;
	u8 sealed_case : 1;
	u8 headless : 1;
	u8 slp_instruction : 1;
	u8 pci_wake_support : 1;
	u8 use_platform_clock : 1;
	u8 rtc_valid_S4 : 1;
	u8 remote_on_support : 1;
	u8 force_apic_cluster : 1;
	u8 force_apic_physical : 1;
	u8 hw_reduced_acpi : 1;
	u8 low_power_S0_support : 1;
	u16 reserved : 10;
};

struct rsdp {
	char signature[8];
	char checksum;
	char oem_id[6];
	char revision;
	u32 *rsdt_address;
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

struct rsdt {
	struct sdt_header header;
	u32 sdt_pointer[];
};

struct xsdt {
	struct sdt_header header;
	u32 sdt_pointer[];
};

struct fadt {
	struct sdt_header header;
	u32 firmware_ctl;
	u32 dsdt;
	u8 reserved;
	u8 preferred_power_management;
	u16 sci_interrupt;
	u32 smi_commandPort;
	u8 acpi_enable;
	u8 acpi_disable;
	u8 S4BIOS_req;
	u8 PSTATE_control;
	u32 PM1a_event_block;
	u32 PM1b_event_block;
	u32 PM1a_control_block;
	u32 PM1b_control_block;
	u32 PM2_control_block;
	u32 PM_timer_block;
	u32 GPE0_block;
	u32 GPE1_block;
	u8 PM1_event_length;
	u8 PM1_control_length;
	u8 PM2_control_length;
	u8 PM_timer_length;
	u8 GPE0_length;
	u8 GPE1_length;
	u8 GPE1_base;
	u8 C_state_control;
	u16 worst_C2_latency;
	u16 worst_C3_latency;
	u16 flush_size;
	u16 flush_stride;
	u8 duty_offset;
	u8 duty_width;
	u8 day_alarm;
	u8 month_alarm;
	u8 century;
	u16 boot_architecture_flags; // Reserved in 1.0
	u8 reserved2;
	struct fadt_flags flags;
	struct address_structure reset_reg;
	u8 reset_value;
	u8 reserved3[3];
	u64 x_firmware_control; // Reserved in 1.0
	u64 x_dsdt; // Reserved in 1.0
	struct address_structure x_PM1a_event_block;
	struct address_structure x_PM1b_event_block;
	struct address_structure x_PM1a_control_block;
	struct address_structure x_PM1b_control_block;
	struct address_structure x_PM2_control_block;
	struct address_structure x_PM_timer_block;
	struct address_structure x_GPE0_block;
	struct address_structure x_GPE1_block;
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

struct madt {
	struct sdt_header header;
	u32 address;
	u32 flags;
	// Interrupt devices...
};

struct rsdt *rsdt;
struct fadt *fadt;
struct hpet *hpet;
struct madt *madt;

#endif
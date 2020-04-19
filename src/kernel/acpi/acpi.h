#ifndef MELVIX_ACPI_H
#define MELVIX_ACPI_H

#include <stdint.h>
#include <kernel/multiboot.h>

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
	uint8_t address_space_id;
	uint8_t register_bit_width;
	uint8_t register_bit_offset;
	uint8_t reserved;
	uint64_t address;
};

// p. 138
struct fadt_flags {
	uint8_t WBINVD : 1;
	uint8_t WBINVD_flush : 1;
	uint8_t C1_support : 1;
	uint8_t C2_mp_support : 1;
	uint8_t power_button : 1; // 1 if not present
	uint8_t sleep_button : 1; // 1 if not present
	uint8_t rtc_fix_reg : 1;
	uint8_t rtc_wakes_S4 : 1;
	uint8_t timer_32 : 1;
	uint8_t dock_support : 1;
	uint8_t reset_support : 1;
	uint8_t sealed_case : 1;
	uint8_t headless : 1;
	uint8_t slp_instruction : 1;
	uint8_t pci_wake_support : 1;
	uint8_t use_platform_clock : 1;
	uint8_t rtc_valid_S4 : 1;
	uint8_t remote_on_support : 1;
	uint8_t force_apic_cluster : 1;
	uint8_t force_apic_physical : 1;
	uint8_t hw_reduced_acpi : 1;
	uint8_t low_power_S0_support : 1;
	uint16_t reserved : 10;
};

struct rsdp {
	char signature[8];
	char checksum;
	char oem_id[6];
	char revision;
	uint32_t *rsdt_address;
};

struct sdt_header {
	char signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	char oem_id[6];
	char oem_table_id[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;
};

struct rsdt {
	struct sdt_header header;
	uint32_t sdt_pointer[];
};

struct xsdt {
	struct sdt_header header;
	uint32_t sdt_pointer[];
};

struct fadt {
	struct sdt_header header;
	uint32_t firmware_ctl;
	uint32_t dsdt;
	uint8_t reserved;
	uint8_t preferred_power_management;
	uint16_t sci_interrupt;
	uint32_t smi_commandPort;
	uint8_t acpi_enable;
	uint8_t acpi_disable;
	uint8_t S4BIOS_req;
	uint8_t PSTATE_control;
	uint32_t PM1a_event_block;
	uint32_t PM1b_event_block;
	uint32_t PM1a_control_block;
	uint32_t PM1b_control_block;
	uint32_t PM2_control_block;
	uint32_t PM_timer_block;
	uint32_t GPE0_block;
	uint32_t GPE1_block;
	uint8_t PM1_event_length;
	uint8_t PM1_control_length;
	uint8_t PM2_control_length;
	uint8_t PM_timer_length;
	uint8_t GPE0_length;
	uint8_t GPE1_length;
	uint8_t GPE1_base;
	uint8_t C_state_control;
	uint16_t worst_C2_latency;
	uint16_t worst_C3_latency;
	uint16_t flush_size;
	uint16_t flush_stride;
	uint8_t duty_offset;
	uint8_t duty_width;
	uint8_t day_alarm;
	uint8_t month_alarm;
	uint8_t century;
	uint16_t boot_architecture_flags; // Reserved in 1.0
	uint8_t reserved2;
	struct fadt_flags flags;
	struct address_structure reset_reg;
	uint8_t reset_value;
	uint8_t reserved3[3];
	uint64_t x_firmware_control; // Reserved in 1.0
	uint64_t x_dsdt; // Reserved in 1.0
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
	uint8_t hardware_rev_id;
	uint8_t comparator_count : 5;
	uint8_t counter_size : 1;
	uint8_t reserved : 1;
	uint8_t legacy_replacement : 1;
	uint16_t pci_vendor_id;
	struct address_structure address;
	uint8_t hpet_number;
	uint16_t minimum_tick;
	uint8_t page_protection;
};

struct madt {
	struct sdt_header header;
	uint32_t address;
	uint32_t flags;
	// Interrupt devices...
};

struct rsdt *rsdt;
struct fadt *fadt;
struct hpet *hpet;
struct madt *madt;

#endif
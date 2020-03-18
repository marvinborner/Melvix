#ifndef MELVIX_ACPI_H
#define MELVIX_ACPI_H

/**
 * Initialize the ACP interface
 * @return 0 if successful, otherwise -1
 */
int acpi_install();

/**
 * Activate a ACPI based device reboot
 */
void reboot();

/**
 * Activate a ACPI based device shutdown/poweroff
 */
void acpi_poweroff();

struct RSD_ptr {
	char signature[8];
	char checksum;
	char oem_id[6];
	char revision;
	uint32_t *rsdt_address;
};

struct FADT {
	char signature[4];
	uint32_t length;
	char unneded1[40 - 8];
	uint32_t *DSDT;
	char unneded2[48 - 44];
	uint32_t *SMI_CMD;
	char ACPI_ENABLE;
	char ACPI_DISABLE;
	char unneded3[64 - 54];
	uint32_t *PM1a_CNT_BLK;
	uint32_t *PM1b_CNT_BLK;
	char unneded4[89 - 72];
	char PM1_CNT_LEN;
	char unneeded5[18];
	char century;
};

struct address_structure {
	uint8_t address_space_id;
	uint8_t register_bit_width;
	uint8_t register_bit_offset;
	uint8_t reserved;
	uint64_t address;
} __attribute__((packed));

struct HPET {
	char signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	char oemid[6];
	uint64_t oem_tableid;
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;
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
} __attribute__((packed));

struct FADT *fadt;

struct HPET *hpet;

#endif

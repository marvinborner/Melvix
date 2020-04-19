#ifndef MELVIX_SMBIOS_H
#define MELVIX_SMBIOS_H

#include <stdint.h>
#include <kernel/multiboot.h>

struct smbios_0 {
	char *vendor;
	char *bios_version;
	uint16_t bios_start;
	char *bios_release_data;
	uint8_t bios_size;
	uint64_t bios_characteristics;
};

struct smbios_1 {
	char *manufacturer;
	char *product_name;
	char *version;
	char *serial_number;
	uint8_t uuid[16];
	uint8_t wakeup_type;
};

void smbios_init(struct multiboot_tag_smbios *tag);

#endif
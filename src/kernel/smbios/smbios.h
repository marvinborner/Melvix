#ifndef MELVIX_SMBIOS_H
#define MELVIX_SMBIOS_H

#include <multiboot.h>
#include <stdint.h>

struct smbios_0 {
	char *vendor;
	char *bios_version;
	u16 bios_start;
	char *bios_release_data;
	u8 bios_size;
	u64 bios_characteristics;
};

struct smbios_1 {
	char *manufacturer;
	char *product_name;
	char *version;
	char *serial_number;
	u8 uuid[16];
	u8 wakeup_type;
};

void smbios_init(struct multiboot_tag_smbios *tag);

#endif
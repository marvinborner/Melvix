#ifndef MELVIX_SMBIOS_H
#define MELVIX_SMBIOS_H

struct smbios_header {
	uint8_t type;
	uint8_t length;
	uint16_t handle;
};

struct smbios_entry {
	int8_t signature[4];
	uint8_t checksum;
	uint8_t length;
	uint8_t major_version;
	uint8_t minor_version;
	uint8_t max_structure_size;
	int8_t entry_point_revision;
	int8_t formatted_area[5];
	int8_t entry_point_signature[5];
	uint8_t checksum2;
	uint16_t table_length;
	uint32_t table_address;
	uint16_t number_of_structures;
	uint8_t bcd_revision;
};

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

struct smbios_entry *get_smbios();

void smbios_table(struct smbios_header *header);

#endif

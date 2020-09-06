// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef ACPI_H
#define ACPI_H

#include <def.h>

#define RSDP_MAGIC "RSD PTR "
#define RSDT_MAGIC "RSDT"

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

struct rsdp {
	struct sdp_header header;
	struct rsdt *rsdt;
};

void acpi_install();

#endif

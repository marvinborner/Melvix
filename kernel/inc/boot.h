// MIT License, Copyright (c) 2020 Marvin Borner
// This file specifies the structs passed by the bootloader

#ifndef BOOT_H
#define BOOT_H

#include <def.h>

struct vid_info {
	u32 mode;
	u32 *vbe;
};

enum mmap_type {
	MEMORY_AVAILABLE = 1,
	MEMORY_RESERVED,
	MEMORY_ACPI,
	MEMORY_NVS,
	MEMORY_DEFECT,
	MEMORY_DISABLED
};

struct mmap_boot {
	u32 lbase;
	u32 hbase;
	u32 lsize;
	u32 hsize;
	u32 type;
	u32 acpi;
};

struct mem_info {
	struct mmap_boot *start;
	u32 *end;
	u32 size;
};

#endif

/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <print.h>
#include <sys.h>

#include <management/disk/index.h>
#include <parts/mbr.h>

#define MAGIC 0xaa55
#define ENTRY(disk, part) ((disk)*4 + (part))

struct mbr_entry {
	u8 attributes;
	u8 chs_start[3];
	u8 type;
	u8 chs_end[3];
	u32 start;
	u32 size;
} PACKED;

struct mbr {
	u8 bootstrap[440];
	u32 signature;
	u16 reserved;
	struct mbr_entry entries[4];
	u16 magic;
} PACKED;

static struct mbr_entry parts[32] = { 0 };

static struct mbr_entry *get(u32 disk, u32 part)
{
	if (!disk || !part)
		return NULL;

	u32 idx = ENTRY(disk, part);
	if (idx >= COUNT(parts))
		return NULL;

	return &parts[idx];
}

static void add(u32 disk, u32 part, struct mbr_entry *entry)
{
	u32 idx = ENTRY(disk, part);
	if (idx >= COUNT(parts))
		panic("No more MBR parts left");
	parts[idx] = *entry;
}

static err read(u32 disk, u32 part, void *buf, u32 offset, u32 count)
{
	struct mbr_entry *entry = get(disk, part);
	if (!entry)
		return -ERR_NOT_FOUND;
	return disk_read(disk, part, buf, entry->start + offset, count);
}

static err write(u32 disk, u32 part, const void *buf, u32 offset, u32 count)
{
	struct mbr_entry *entry = get(disk, part);
	if (!entry)
		return -ERR_NOT_FOUND;
	return disk_write(disk, part, buf, entry->start + offset, count);
}

err mbr_load(u32 disk)
{
	struct mbr mbr;
	TRY(disk_read(disk, DISK_RAW, &mbr, 0, 1));

	if (mbr.magic != MAGIC)
		return -ERR_NOT_FOUND;

	for (u8 i = 0; i < 4; i++) {
		struct mbr_entry *entry = &mbr.entries[i];
		if (!entry->size || !entry->type)
			continue;

		struct part part = { .read = read, .write = write };
		u32 id = disk_part_add(disk, &part);
		log("MBR: Found part -> %d/%d", disk, id);
		add(disk, id, entry);
	}

	return ERR_OK;
}

/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <print.h>

#include <management/disk/index.h>
#include <parts/gpt.h>
#include <parts/mbr.h>

PROTECTED static struct disk disks[64] = { 0 };

void disk_delete(u32 id)
{
	if (id >= COUNT(disks) + 1 || !id || disks[id - 1].id != id)
		return;

	disks[id - 1].id = 0;
}

u32 disk_add(struct disk *disk)
{
	disk->id = 0;
	for (u32 i = 0; i < COUNT(disks); i++) {
		if (!disks[i].id) {
			disk->id = i + 1;
			break;
		}
	}

	if (!disk->id)
		panic("No disks left");

	if (!disk->read || !disk->write)
		panic("Disk %d isn't writable/readable", disk->id);

	disks[disk->id - 1] = *disk;
	return disk->id;
}

u32 disk_part_add(u32 id, struct part *part)
{
	struct disk *disk = disk_get(id);
	if (!disk)
		panic("Disk %d not found", disk->id);

	part->id = 0;
	for (u32 i = 0; i < COUNT(disk->parts); i++) {
		if (!disk->parts[i].id) {
			part->id = i + 1;
			break;
		}
	}

	if (!part->id)
		panic("No partitions left");

	disk->parts[part->id - 1] = *part;
	return part->id;
}

struct disk *disk_get(u32 id)
{
	if (id >= COUNT(disks) + 1 || !id || disks[id - 1].id != id)
		return NULL;
	return &disks[id - 1];
}

void disk_iterate(err (*callback)(u32))
{
	for (u32 i = 0; i < COUNT(disks); i++)
		if (disks[i].id)
			callback(disks[i].id);
}

/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <print.h>

#include <management/disk/index.h>

PROTECTED static struct disk disks[64] = { 0 };

void disk_delete(u32 id)
{
	if (id >= COUNT(disks) + 1 || !id || disks[id - 1].id != id)
		return;

	disks[id - 1].id = 0;
}

u32 disk_add(struct disk *disk)
{
	for (u32 i = 0; i < COUNT(disks); i++) {
		if (!disks[i].id) {
			disk->id = i + 1;
			break;
		}
	}

	if (!disk->read || !disk->write)
		panic("Disk %d isn't writable/readable", disk->id);

	disks[disk->id - 1] = *disk;
	return disk->id;
}

struct disk *disk_get(u32 id)
{
	if (id >= COUNT(disks) + 1 || !id || disks[id - 1].id != id)
		return NULL;
	return &disks[id];
}

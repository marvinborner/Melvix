/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#ifndef MANAGEMENT_DISK_INDEX_H
#define MANAGEMENT_DISK_INDEX_H

#include <err.h>

typedef err (*part_read_t)(u32 disk, u32 part, void *buf, u32 offset, u32 count);
typedef err (*part_write_t)(u32 disk, u32 part, const void *buf, u32 offset, u32 count);

struct part {
	u32 id;
	part_read_t read;
	part_write_t write;
};

typedef err (*disk_read_t)(u32 disk, void *buf, u32 offset, u32 count);
typedef err (*disk_write_t)(u32 disk, const void *buf, u32 offset, u32 count);

struct disk {
	u32 id;
	disk_read_t read;
	disk_write_t write;
	struct part parts[8];
};

void disk_delete(u32 id);
NONNULL u32 disk_add(struct disk *disk);
NONNULL u32 disk_part_add(u32 id, struct part *part);
struct disk *disk_get(u32 id);
NONNULL void disk_iterate(err (*callback)(u32));

#endif

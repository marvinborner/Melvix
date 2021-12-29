/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#ifndef MANAGEMENT_DISK_INDEX_H
#define MANAGEMENT_DISK_INDEX_H

#include <arg.h>
#include <sys.h>

typedef err (*disk_read_t)(u32 id, void *buf, u32 offset, u32 count);
typedef err (*disk_write_t)(u32 id, const void *buf, u32 offset, u32 count);

struct disk {
	u32 id;
	disk_read_t read;
	disk_write_t write;
};

void disk_delete(u32 id);
NONNULL u32 disk_add(struct disk *disk);
struct disk *disk_get(u32 id);

#endif

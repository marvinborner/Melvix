/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <sys.h>

#include <management/disk/index.h>
#include <parts/gpt.h>
#include <parts/mbr.h>

err disk_read(u32 id, u32 part, void *buf, u32 offset, u32 count)
{
	struct disk *disk = disk_get(id);
	if (!disk)
		return -ERR_INVALID_ARGUMENTS;

	if (part == DISK_RAW) {
		if (!disk->read)
			return -ERR_NOT_SUPPORTED;
		return disk->read(id, buf, offset, count);
	}

	if (!part || part >= COUNT(disk->parts))
		return -ERR_INVALID_ARGUMENTS;
	if (!disk->parts[part - 1].read)
		return -ERR_NOT_SUPPORTED;
	return disk->parts[part - 1].read(id, part, buf, offset, count);
}

err disk_write(u32 id, u32 part, const void *buf, u32 offset, u32 count)
{
	struct disk *disk = disk_get(id);
	if (!disk)
		return -ERR_INVALID_ARGUMENTS;

	if (part == DISK_RAW) {
		if (!disk->write)
			return -ERR_NOT_SUPPORTED;
		return disk->write(id, buf, offset, count);
	}

	if (!part || part >= COUNT(disk->parts))
		return -ERR_INVALID_ARGUMENTS;
	if (!disk->parts[part - 1].write)
		return -ERR_NOT_SUPPORTED;
	return disk->parts[part - 1].write(id, part, buf, offset, count);
}

err disk_load(u32 id)
{
	err load = mbr_load(id);
	// TODO: GPT
	/* if (load != ERR_OK) */
	/* 	load = gpt_load(id); */
	return load;
}

/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <management/disk/index.h>

err disk_read(u32 id, void *buf, u32 offset, u32 count)
{
	struct disk *disk = disk_get(id);
	if (!disk)
		return -ERR_INVALID_ARGUMENTS;
	if (!disk->read)
		return -ERR_NOT_SUPPORTED;
	return disk->read(id, buf, offset, count);
}

err disk_write(u32 id, const void *buf, u32 offset, u32 count)
{
	struct disk *disk = disk_get(id);
	if (!disk)
		return -ERR_INVALID_ARGUMENTS;
	if (!disk->write)
		return -ERR_NOT_SUPPORTED;
	return disk->write(id, buf, offset, count);
}

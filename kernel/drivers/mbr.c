// MIT License, Copyright (c) 2021 Marvin Borner

#include <def.h>
#include <fs.h>
#include <ide.h>
#include <mbr.h>
#include <mem.h>
#include <print.h>
#include <str.h>

struct mbr_data {
	struct mbr_entry mbr;
	struct vfs_dev *dev;
};

static res mbr_read(void *buf, u32 lba, u32 sector_count, struct vfs_dev *part)
{
	struct mbr_data *data = part->data;
	if (!data || !data->dev || !data->dev->read)
		return -EINVAL;

	return data->dev->read(buf, data->mbr.start + lba, sector_count, data->dev);
}

static res mbr_write(const void *buf, u32 lba, u32 sector_count, struct vfs_dev *part)
{
	struct mbr_data *data = part->data;
	if (!data || !data->dev || !data->dev->write)
		return -EINVAL;

	return data->dev->write(buf, data->mbr.start + lba, sector_count, data->dev);
}

CLEAR u8 mbr_load(struct vfs_dev *dev)
{
	struct mbr mbr = { 0 };
	dev->read(&mbr, 0, 1, dev); // Read first sector

	if (mbr.magic != 0xaa55)
		return 0;

	for (u8 i = 0; i < 4; i++) {
		struct mbr_entry *entry = &mbr.entries[i];
		if (!entry->type || !entry->size)
			continue;

		struct mbr_data *data = zalloc(sizeof(*data));
		memcpy(&data->mbr, entry, sizeof(*entry));
		data->dev = dev;

		struct vfs_dev *part = zalloc(sizeof(*part));
		part->data = data;
		strlcpy(part->name, dev->name, 4);
		part->name[3] = '0' + i;
		part->type = DEV_BLOCK;
		part->read = mbr_read;
		part->write = mbr_write;

		/* printf("%s [start: %d; size: %dMiB; type: %d]\n", part->name, entry->start, */
		/*        (entry->size * SECTOR_SIZE) >> 20, entry->type); */

		vfs_add_dev(part);
		vfs_load(part);
	}

	return 1;
}

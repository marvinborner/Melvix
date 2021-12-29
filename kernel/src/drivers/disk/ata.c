/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <print.h>

#include <drivers/disk.h>
#include <management/device/index.h>
#include <management/disk/index.h>
#include <ports/ata.h>

#define PRIMARY 0x00
#define SECONDARY 0x01
#define MASTER 0x00
#define SLAVE 0x01

struct ata_disk {
	u32 id;
	u8 bus, drive;
};

static struct ata_disk disks[4] = { 0 };

static struct ata_disk *get(u32 id)
{
	if (!id)
		return NULL;

	for (u8 i = 0; i < 4; i++)
		if (disks[i].id == id)
			return &disks[i];
	return NULL;
}

static err read(u32 id, void *buf, u32 offset, u32 count)
{
	struct ata_disk *disk = get(id);
	if (!disk)
		return -ERR_NOT_FOUND;
	return port_request(PORT_ATA, PORT_ATA_READ, disk->bus, disk->drive, buf, offset, count);
}

static err write(u32 id, const void *buf, u32 offset, u32 count)
{
	UNUSED(id);
	UNUSED(buf);
	UNUSED(offset);
	UNUSED(count);
	return -ERR_NOT_SUPPORTED; // TODO
}

static err enable(void)
{
	TRY(port_setup(PORT_ATA));

	for (u8 i = 0; i < 4; i++) {
		u8 bus = i < 2 ? PRIMARY : SECONDARY;
		u8 drive = i % 2 ? MASTER : SLAVE;

		if (port_request(PORT_ATA, PORT_ATA_EXISTS, bus, drive) != ERR_OK)
			continue;

		struct disk disk = { .read = read, .write = write };
		u32 id = disk_add(&disk);
		disks[i] = (struct ata_disk){ .id = id, .bus = bus, .drive = drive };
		log("ATA: Found hd%c -> %d", 'a' + i, id);
	}

	return ERR_OK;
}

static err disable(void)
{
	for (u8 i = 0; i < 4; i++)
		disk_delete(disks[i].id);

	return ERR_OK;
}

static err probe(void)
{
	TRY(port_probe(PORT_ATA));

	return ERR_OK;
}

PROTECTED struct device device_disk = {
	.type = DEVICE_DISK,
	.enable = enable,
	.disable = disable,
	.probe = probe,
};

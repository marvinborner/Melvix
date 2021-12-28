/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <print.h>

#include <drivers/disk.h>
#include <management/device/index.h>
#include <ports/ata.h>

#define PRIMARY 0x00
#define SECONDARY 0x01
#define MASTER 0x00
#define SLAVE 0x01

static err enable(void)
{
	TRY(port_setup(PORT_ATA));

	for (u8 i = 0; i < 4; i++) {
		u8 bus = i < 2 ? PRIMARY : SECONDARY;
		u8 drive = i % 2 ? MASTER : SLAVE;

		if (port_request(PORT_ATA, PORT_ATA_EXISTS, bus, drive) != ERR_OK)
			continue;

		log("ATA: Found hd%c", 'a' + i);
	}

	// TODO: Add disk to disk management

	return ERR_OK;
}

static err disable(void)
{
	// TODO: Remove disk from disk management
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

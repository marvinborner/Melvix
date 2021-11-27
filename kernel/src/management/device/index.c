/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <def.h>
#include <print.h>

#include <management/device/index.h>

PROTECTED static struct device *devices[DEVICE_MAX] = { 0 };

TEMPORARY void device_add(struct device *device)
{
	if (!device->enable || !device->disable)
		panic("Device can't be enabled/disabled");
	if (!device->probe || device->probe() != ERR_OK)
		panic("Device probing failed");

	if (devices[device->type])
		devices[device->type]->disable();

	devices[device->type] = device;
	if (device->enable() != ERR_OK)
		log("Could not enable device %d", device->type);
}

struct device *device_get(enum device_type type)
{
	if (type >= DEVICE_MAX || !devices[type] || devices[type]->type != type)
		return NULL;
	return devices[type];
}

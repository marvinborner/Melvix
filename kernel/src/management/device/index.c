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
		panic("Device %d can't be enabled/disabled", device->type);
	if (!device->probe)
		panic("Device %d can't be probed", device->type);

	u8 probe = device->probe();
	if (probe != ERR_OK)
		panic("Device %d probe failed: %e", device->type, probe);

	if (devices[device->type])
		devices[device->type]->disable();

	devices[device->type] = device;
	u8 enable = device->enable();
	if (enable != ERR_OK)
		log("Device %d could not get enabled: %e", device->type, enable);
}

struct device *device_get(enum device_type type)
{
	if (type >= DEVICE_MAX || !devices[type] || devices[type]->type != type)
		return NULL;
	return devices[type];
}

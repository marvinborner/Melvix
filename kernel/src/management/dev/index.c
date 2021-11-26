/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <def.h>
#include <print.h>

#include <management/dev/index.h>

PROTECTED static struct dev *devs[DEV_MAX] = { 0 };

TEMPORARY void dev_add(struct dev *dev)
{
	if (!dev->enable || !dev->disable)
		panic("Device can't be enabled/disabled\n");
	else if (!dev->probe || dev->probe() != ERR_OK)
		panic("Device probing failed\n");

	if (devs[dev->type])
		devs[dev->type]->disable();

	devs[dev->type] = dev;
	dev->enable();
}

struct dev *dev_get(enum dev_type type)
{
	if (type >= DEV_MAX || !devs[type] || devs[type]->type != type)
		return NULL;
	return devs[type];
}

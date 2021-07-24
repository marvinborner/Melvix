// MIT License, Copyright (c) 2021 Marvin Borner

#include <dev/management.h>

PROTECTED static struct dev devs[DEV_MAX] = { 0 };

CLEAR void dev_add(struct dev *dev)
{
	if (!dev->enable || !dev->disable)
		kernel_panic("Device can't be enabled/disabled\n");

	if (devs[dev->type].exists)
		devs[dev->type].disable();

	dev->exists = 1;
	devs[dev->type] = *dev;
	dev->enable();
}

struct dev *dev_get(dev_t type)
{
	if (type >= DEV_MAX)
		return NULL;
	return &devs[type];
}

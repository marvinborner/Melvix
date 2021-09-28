// MIT License, Copyright (c) 2021 Marvin Borner

#include <def.h>
#include <print.h>

#include <management/dev/index.h>

PROTECTED static struct dev devs[DEV_MAX] = { 0 };

TEMPORARY void dev_add(struct dev *dev)
{
	if (!dev->enable || !dev->disable)
		panic("Device can't be enabled/disabled\n");

	if (devs[dev->type].exists)
		devs[dev->type].disable();

	dev->exists = 1;
	devs[dev->type] = *dev;
	dev->enable();
}

struct dev *dev_get(enum dev_type type)
{
	if (type >= DEV_MAX)
		return NULL;
	return &devs[type];
}

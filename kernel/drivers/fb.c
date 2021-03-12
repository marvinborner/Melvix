// MIT License, Copyright (c) 2021 Marvin Borner

#include <def.h>
#include <fb.h>
#include <fs.h>
#include <ioctl.h>
#include <mem.h>
#include <str.h>
#include <sys.h>

static u32 dev_id = 0;
static struct vid_info *info = NULL;

static s32 fb_ioctl(u32 request, void *arg1, void *arg2, void *arg3, struct device *dev)
{
	UNUSED(arg2);
	UNUSED(arg3);
	UNUSED(dev);

	switch (request) {
	case IO_FB_GET:
		memcpy(arg1, info->vbe, 256);
		return 0;
	default:
		return -1;
	}
}

static u8 fb_ready(void)
{
	return 1;
}

void fb_install(struct vid_info *boot)
{
	info = boot;

	struct device *dev = zalloc(sizeof(*dev));
	dev->name = strdup("fb");
	dev->type = DEV_CHAR;
	dev->ioctl = fb_ioctl;
	dev->ready = fb_ready;
	device_add(dev);
	dev_id = dev->id;
}

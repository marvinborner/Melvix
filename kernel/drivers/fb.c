// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <def.h>
#include <errno.h>
#include <fb.h>
#include <fs.h>
#include <ioctl.h>
#include <mem.h>
#include <mm.h>
#include <str.h>
#include <sys.h>

struct vbe_basic {
	u8 stuff1[16];
	u16 pitch;
	u16 width;
	u16 height;
	u8 stuff2[18];
	u8 *fb;
	u8 stuff3[212];
};

static u32 dev_id = 0;
static struct vid_info *info = NULL;

static u32 fb_owner = 0;
static res fb_ioctl(u32 request, void *arg1, void *arg2, void *arg3, struct device *dev)
{
	UNUSED(arg2);
	UNUSED(arg3);
	UNUSED(dev);

	switch (request) {
	case IO_FB_GET: {
		if (!info)
			return -ENOENT;

		if (!arg1 || !memory_valid(arg1))
			return -EFAULT;

		if (fb_owner != 0 && proc_from_pid(fb_owner))
			return -EBUSY;
		else
			fb_owner = proc_current()->pid;

		memcpy(arg1, info->vbe, sizeof(struct vbe_basic));
		fb_map_buffer(proc_current()->page_dir, info);
		return EOK;
	}
	default:
		return -EINVAL;
	}
}

static res fb_ready(void)
{
	return 1;
}

void fb_map_buffer(struct page_dir *dir, struct vid_info *boot)
{
	struct vbe_basic *vbe = (struct vbe_basic *)boot->vbe;
	u32 size = vbe->height * vbe->pitch;
	memory_map_identity(dir, memory_range_around((u32)vbe->fb, size), MEMORY_USER);
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

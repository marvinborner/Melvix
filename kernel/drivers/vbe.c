// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <def.h>
#include <dev.h>
#include <drivers/cpu.h>
#include <drivers/vbe.h>
#include <errno.h>
#include <fb.h>
#include <mem.h>
#include <mm.h>
#include <multiboot.h>
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

PROTECTED static struct fb_generic generic = { 0 };

static u32 fb_owner = 0;
static res vbe_control(u32 request, void *arg1, void *arg2, void *arg3)
{
	UNUSED(arg3);

	switch (request) {
	case DEVCTL_FB_GET: {
		if (!generic.fb)
			return -ENOENT;

		u32 size = MIN(sizeof(generic), (u32)arg2);
		if (!memory_writable_range(memory_range(arg1, size)))
			return -EFAULT;

		if (fb_owner != 0 && proc_from_pid(fb_owner))
			return -EBUSY;
		fb_owner = proc_current()->pid;

		u32 fb = fb_map_buffer(proc_current()->page_dir, &generic);

		stac();
		memcpy(arg1, &generic, size);
		((struct fb_generic *)arg1)->fb = (u8 *)fb;
		clac();

		return EOK;
	}
	default:
		return -EINVAL;
	}
}

CLEAR void vbe_install(u32 data)
{
	struct vbe_basic *vbe = (void *)data;
	generic.bpp = (vbe->pitch / vbe->width) << 3;
	generic.pitch = vbe->pitch;
	generic.width = vbe->width;
	generic.height = vbe->height;
	generic.fb = vbe->fb;
	fb_protect(&generic);

	struct dev_dev *dev = zalloc(sizeof(*dev));
	dev->control = vbe_control;
	dev_add(DEV_FRAMEBUFFER, dev);
}

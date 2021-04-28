// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <cpu.h>
#include <def.h>
#include <errno.h>
#include <fb.h>
#include <io.h>
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

PROTECTED static struct vid_info *info = NULL;

static u32 fb_owner = 0;
static res fb_ioctl(u32 request, void *arg1, void *arg2, void *arg3)
{
	UNUSED(arg2);
	UNUSED(arg3);

	switch (request) {
	case IOCTL_FB_GET: {
		if (!info)
			return -ENOENT;

		if (!memory_writable_range(memory_range(arg1, sizeof(struct vbe_basic))))
			return -EFAULT;

		if (fb_owner != 0 && proc_from_pid(fb_owner))
			return -EBUSY;
		else
			fb_owner = proc_current()->pid;

		stac();
		memcpy(arg1, info->vbe, sizeof(struct vbe_basic));
		clac();
		fb_map_buffer(proc_current()->page_dir, info);
		return EOK;
	}
	default:
		return -EINVAL;
	}
}

void fb_map_buffer(struct page_dir *dir, struct vid_info *boot)
{
	struct vbe_basic *vbe = (struct vbe_basic *)boot->vbe;
	u32 size = vbe->height * vbe->pitch;
	memory_map_identity(dir, memory_range_around((u32)vbe->fb, size), MEMORY_USER);
}

CLEAR void fb_install(void)
{
	//info = boot;

	struct io_dev *dev = zalloc(sizeof(*dev));
	dev->control = fb_ioctl;
	io_add(IO_FRAMEBUFFER, dev);
}

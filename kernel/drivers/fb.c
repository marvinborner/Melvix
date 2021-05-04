// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <cpu.h>
#include <def.h>
#include <errno.h>
#include <fb.h>
#include <io.h>
#include <mem.h>
#include <mm.h>
#include <multiboot.h>
#include <str.h>
#include <sys.h>

#define FB_SIZE (vbe->height * vbe->pitch)

struct vbe_basic {
	u8 stuff1[16];
	u16 pitch;
	u16 width;
	u16 height;
	u8 stuff2[18];
	u8 *fb;
	u8 stuff3[212];
};

PROTECTED static struct vbe_basic *vbe = NULL;

static u32 fb_map_buffer(struct page_dir *dir)
{
	assert(vbe);
	return virtual_alloc(dir, memory_range_around((u32)vbe->fb, FB_SIZE), MEMORY_USER).base;
}

static u32 fb_owner = 0;
static res fb_ioctl(u32 request, void *arg1, void *arg2, void *arg3)
{
	UNUSED(arg2);
	UNUSED(arg3);

	switch (request) {
	case IOCTL_FB_GET: {
		if (!vbe)
			return -ENOENT;

		u32 size = MIN(sizeof(*vbe), (u32)arg2);
		if (!memory_writable_range(memory_range(arg1, size)))
			return -EFAULT;

		if (fb_owner != 0 && proc_from_pid(fb_owner))
			return -EBUSY;
		fb_owner = proc_current()->pid;

		u32 fb = fb_map_buffer(proc_current()->page_dir);
		vbe->fb = (u8 *)fb;
		memcpy_user(arg1, vbe, size);
		return EOK;
	}
	default:
		return -EINVAL;
	}
}

CLEAR void fb_install(void)
{
	vbe = (void *)multiboot_vbe();

	struct io_dev *dev = zalloc(sizeof(*dev));
	dev->control = fb_ioctl;
	io_add(IO_FRAMEBUFFER, dev);

	// Set framebuffer range used to prevent unwanted writing
	physical_set_used(memory_range_around((u32)vbe->fb, FB_SIZE));
}

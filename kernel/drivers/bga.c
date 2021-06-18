// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <def.h>
#include <dev.h>
#include <drivers/bga.h>
#include <drivers/cpu.h>
#include <drivers/pci.h>
#include <fb.h>
#include <mem.h>
#include <mm.h>

#define BGA_ADDRESS 0x01ce
#define BGA_DATA 0x01cf

#define BGA_VERSION 0
#define BGA_XRES 1
#define BGA_YRES 2
#define BGA_BPP 3
#define BGA_ENABLE 4
#define BGA_BANK 5
#define BGA_VWIDTH 6
#define BGA_VHEIGHT 7
#define BGA_XOFF 8
#define BGA_YOFF 9

#define BGA_LINEAR_FB 0x40

#define BGA_V0 0xb0c0
#define BGA_V1 0xb0c1
#define BGA_V2 0xb0c2
#define BGA_V3 0xb0c3
#define BGA_V4 0xb0c4
#define BGA_V5 0xb0c5

PROTECTED static u32 bga_device_pci = 0;
PROTECTED static struct fb_generic generic = { 0 };

CLEAR static void bga_find(u32 device, u16 vendor_id, u16 device_id, void *extra)
{
	if ((vendor_id == 0x1234) && (device_id == 0x1111))
		*((u32 *)extra) = device;
}

static void bga_write_reg(u16 address, u16 data)
{
	outw(BGA_ADDRESS, address);
	outw(BGA_DATA, data);
}

static u16 bga_read_reg(u16 index)
{
	outw(BGA_ADDRESS, index);
	return inw(BGA_DATA);
}

CLEAR u8 bga_available(void)
{
	pci_scan(&bga_find, -1, &bga_device_pci);
	u16 status = bga_read_reg(BGA_VERSION);
	return bga_device_pci != 0 && status >= BGA_V0 && status <= BGA_V5;
}

CLEAR static u32 bga_fb_base(void)
{
	u32 bar0 = pci_read_field(bga_device_pci, PCI_BAR0, 4);
	assert(!(bar0 & 7)); // MMIO32
	return bar0 & 0xfffffff0;
}

// TODO: BGA resolution using control calls
static u32 fb_owner = 0;
static res bga_control(u32 request, void *arg1, void *arg2, void *arg3)
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

CLEAR static void bga_enable(u16 width, u16 height, u16 depth)
{
	bga_write_reg(BGA_ENABLE, 0);
	bga_write_reg(BGA_XRES, width);
	bga_write_reg(BGA_YRES, height);
	bga_write_reg(BGA_BPP, depth);
	bga_write_reg(BGA_ENABLE, 1 | BGA_LINEAR_FB);

	generic.pitch = width * (depth >> 3);
	generic.bpp = depth;
	generic.width = width;
	generic.height = height;
	generic.fb = (u8 *)bga_fb_base();
	fb_protect(&generic);
}

CLEAR void bga_install(void)
{
	bga_enable(1920, 1200, 32);

	struct dev_dev *dev = zalloc(sizeof(*dev));
	dev->control = bga_control;
	dev_add(DEV_FRAMEBUFFER, dev);
}

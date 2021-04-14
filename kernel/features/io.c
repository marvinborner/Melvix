// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <def.h>
#include <fb.h>
#include <io.h>
#include <list.h>
#include <mm.h>
#include <ps2.h>
#include <rand.h>
#include <str.h>
#include <timer.h>

PROTECTED static struct io_dev *io_mappings[IO_MAX] = { 0 };

static u8 io_type_valid(enum io_type io)
{
	return io > IO_MIN && io < IO_MAX;
}

static struct io_dev *io_get(enum io_type io)
{
	if (!io_type_valid(io))
		return NULL;

	return io_mappings[io];
}

CLEAR void io_add(enum io_type io, struct io_dev *dev)
{
	assert(io_type_valid(io) && !io_mappings[io]);
	io_mappings[io] = dev;
}

res io_poll(u32 *devs)
{
	if (!memory_readable(devs))
		return -EFAULT;

	for (u32 *p = devs; p && memory_readable(p) && *p; p++) {
		if (!io_get(*p))
			return -ENOENT;
	}

	return -EFAULT;
}

res io_control(enum io_type io, u32 request, void *arg1, void *arg2, void *arg3)
{
	struct io_dev *dev;
	if (!(dev = io_get(io)) || !dev->control)
		return -ENOENT;

	return dev->control(request, arg1, arg2, arg3);
}

res io_write(enum io_type io, void *buf, u32 offset, u32 count)
{
	if (!memory_readable(buf))
		return -EFAULT;

	struct io_dev *dev;
	if (!(dev = io_get(io)) || !dev->write)
		return -ENOENT;

	return dev->write(buf, offset, count);
}

res io_read(enum io_type io, void *buf, u32 offset, u32 count)
{
	if (!memory_readable(buf))
		return -EFAULT;

	struct io_dev *dev;
	if (!(dev = io_get(io)) || !dev->read)
		return -ENOENT;

	return dev->read(buf, offset, count);
}

CLEAR void io_install(struct boot_info *boot)
{
	ps2_detect();

	u8 ps2_keyboard = ps2_keyboard_detect();
	if (ps2_keyboard != U8_MAX) {
		ps2_keyboard_install(ps2_keyboard);
	}

	u8 ps2_mouse = ps2_mouse_detect();
	if (ps2_mouse != U8_MAX) {
		ps2_mouse_install(ps2_mouse);
	}

	timer_install();
	fb_install(boot->vid);
}

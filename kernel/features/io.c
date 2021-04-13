// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <def.h>
#include <io.h>
#include <list.h>
#include <mm.h>
#include <rand.h>
#include <str.h>

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

res io_control(enum io_type io)
{
	if (!io_get(io))
		return -ENOENT;

	return -ENOENT;
}

res io_write(enum io_type io, void *buf, u32 offset, u32 count)
{
	if (!memory_readable(buf))
		return -EFAULT;

	if (!io_get(io))
		return -ENOENT;
}

res io_read(enum io_type io, void *buf, u32 offset, u32 count)
{
	if (!memory_readable(buf))
		return -EFAULT;

	if (!io_get(io))
		return -ENOENT;
}

res io_poll(enum io_type io)
{
	if (!io_get(io))
		return -ENOENT;
}

CLEAR void io_install(void)
{
	// TODO: Install I/O devices by selecting best working driver
}

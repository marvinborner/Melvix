// MIT License, Copyright (c) 2021 Marvin Borner

#include <def.h>
#include <dev.h>
#include <drivers/cpu.h>
#include <drivers/serial.h>
#include <errno.h>
#include <logger.h>
#include <mem.h>
#include <print.h>

static res logger_write(const void *buf, u32 offset, u32 count)
{
	if (offset)
		return -EINVAL;

	if (!count)
		return EOK;

	print_prefix();

	u32 i;
	stac();
	for (i = 0; i < count; i++) {
		if (!((const u8 *)buf)[i])
			break;

		serial_put(((const u8 *)buf)[i]);
	}
	clac();

	serial_print("\x1B[0m");

	return i;
}

void logger_install(void)
{
	struct dev_dev *dev = zalloc(sizeof(*dev));
	dev->write = logger_write;
	dev_add(DEV_LOGGER, dev);
}

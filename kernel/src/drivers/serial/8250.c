/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <def.h>
#include <err.h>

#include <core/io.h>
#include <drivers/serial.h>

#define PORT 0x3f8

static err enable(void)
{
	IO_OUTB(PORT + 1, 0x00);
	IO_OUTB(PORT + 3, 0x80);
	IO_OUTB(PORT + 0, 0x03);
	IO_OUTB(PORT + 1, 0x00);
	IO_OUTB(PORT + 3, 0x03);
	IO_OUTB(PORT + 2, 0xc7);

	IO_OUTB(PORT + 4, 0x0f);
	return ERR_OK;
}

static err disable(void)
{
	IO_OUTB(PORT + 4, 0x1e);
	return ERR_OK;
}

static err probe(void)
{
	enable();

	IO_OUTB(PORT + 4, 0x1e); // Enable loopback
	IO_OUTB(PORT + 0, 0xae); // Write
	if (IO_INB(PORT) == 0xae)
		return ERR_OK;
	return ERR_NOT_FOUND;
}

static void put(char ch)
{
	while (!(IO_INB(PORT + 5) & 0x20))
		;

	IO_OUTB(PORT, (u8)ch);
}

static err write(const void *buf, u32 offset, u32 count)
{
	UNUSED(offset);

	for (const char *p = buf; *p && (u32)(p - (const char *)buf) < count; p++)
		put(*p);
	return ERR_OK;
}

PROTECTED struct dev serial_device = {
	.type = DEV_LOGGER,
	.write = write,
	.enable = enable,
	.disable = disable,
	.probe = probe,
};

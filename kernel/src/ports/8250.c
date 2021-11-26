/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */
// 8250 is a serial IC

#include <err.h>

#include <core/io.h>
#include <ports/8250.h>

#define PORT 0x3f8

static void write_byte(u8 ch)
{
	while (!(IO_INB(PORT + 5) & 0x20))
		;

	IO_OUTB(PORT, ch);
}

static err write(const void *buf, u32 offset, u32 count)
{
	UNUSED(offset);

	for (const u8 *p = buf; *p && (u32)(p - (const u8 *)buf) < count; p++)
		write_byte(*p);
	return count;
}

static err setup(void)
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

static err probe(void)
{
	setup();

	IO_OUTB(PORT + 4, 0x1e); // Enable loopback
	IO_OUTB(PORT + 0, 0xae); // Write
	if (IO_INB(PORT) == 0xae)
		return ERR_OK;
	return ERR_NOT_FOUND;
}

PROTECTED struct port port_8250 = {
	.type = PORT_8250,
	.write = write,
	.probe = probe,
	.setup = setup,
};

/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <err.h>

#include <drivers/serial.h>

static err enable(void)
{
	return port_setup(PORT_8250);
}

static err disable(void)
{
	// TODO: request to loopback
	return ERR_OK;
}

static err probe(void)
{
	return port_probe(PORT_8250);
}

static err write(const void *buf, u32 offset, u32 count)
{
	return port_write(PORT_8250, buf, offset, count);
}

PROTECTED struct dev device_serial = {
	.type = DEV_LOGGER,
	.write = write,
	.enable = enable,
	.disable = disable,
	.probe = probe,
};

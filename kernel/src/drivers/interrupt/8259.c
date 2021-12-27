/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <drivers/interrupt.h>
#include <ports/8259.h>

static err request(u32 request, va_list ap)
{
	switch (request) {
	case DEVICE_INTERRUPT_ACK:
		return port_request(PORT_8259, PORT_8259_ACK, va_arg(ap, u32));
	default:
		return -ERR_INVALID_ARGUMENTS;
	}
}

static err enable(void)
{
	return port_setup(PORT_8259);
}

static err disable(void)
{
	// TODO: Disable port 8259
	return -ERR_NOT_SUPPORTED;
}

static err probe(void)
{
	return port_probe(PORT_8259);
}

PROTECTED struct device device_interrupt = {
	.type = DEVICE_INTERRUPT,
	.request = request,
	.enable = enable,
	.disable = disable,
	.probe = probe,
};

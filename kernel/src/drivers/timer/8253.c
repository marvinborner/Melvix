/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <print.h>

#include <drivers/timer.h>
#include <management/device/index.h>
#include <ports/8253.h>

#define CHANNEL 0 // TODO: For now

static u32 time = 0;

static err get(void)
{
	return time;
}

static err handler(void *data)
{
	UNUSED(data);
	timer_execute(++time);
	return ERR_OK;
}

static err phase(u32 hz)
{
	return port_request(PORT_8253, PORT_8253_PHASE, CHANNEL, hz);
}

static err request(u32 request, va_list ap)
{
	switch (request) {
	case DEVICE_TIMER_GET:
		return get();
	case DEVICE_TIMER_PHASE:
		return phase(va_arg(ap, int));
	default:
		return -ERR_INVALID_ARGUMENTS;
	}
}

static err enable(void)
{
	TRY(port_setup(PORT_8253));
	TRY(phase(1000));

	interrupt_register(0, handler);
	return ERR_OK;
}

static err disable(void)
{
	interrupt_remove(0);
	return ERR_OK;
}

static err probe(void)
{
	TRY(port_probe(PORT_8253));

	return ERR_OK;
}

PROTECTED struct device device_timer = {
	.type = DEVICE_TIMER,
	.request = request,
	.enable = enable,
	.disable = disable,
	.probe = probe,
};

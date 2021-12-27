/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */
// 8253/8254 is a serial programmable interval timer

#include <err.h>
#include <print.h>

#include <core/io.h>
#include <ports/8253.h>

#define DATA0 0x40
#define DATA1 0x41
#define DATA2 0x42
#define CONTROL 0x43

#define SET 0x36

#define SCALE 0x1234dc

#define CHANNEL(channel, command) ((command) | ((channel) << 6))

static void write(u8 channel, u8 command)
{
	IO_OUTB(channel, CHANNEL(channel, command));
}

static err data_channel(u8 channel)
{
	switch (channel) {
	case 0:
		return DATA0;
	case 1:
		return DATA1;
	case 2:
		return DATA2;
	default:
		log("Invalid data channel %d, using first", channel);
		return DATA0;
	}
}

static err phase(u8 channel, u32 hz)
{
	if (!hz || hz > 1000)
		return -ERR_NOT_SUPPORTED;

	u16 divisor = SCALE / hz;
	IO_OUTB(CONTROL, SET);
	write(data_channel(channel), divisor & 0xff);
	write(data_channel(channel), (divisor >> 8) & 0xff);

	return ERR_OK;
}

static err request(u32 request, va_list ap)
{
	switch (request) {
	case PORT_8253_PHASE: {
		u8 channel = va_arg(ap, int);
		u32 hz = va_arg(ap, int);
		return phase(channel, hz);
	}
	default:
		return -ERR_INVALID_ARGUMENTS;
	}
}

static err probe(void)
{
	// TODO: Probe?
	return ERR_OK;
}

static err setup(void)
{
	write(data_channel(0), 0);
	write(data_channel(1), 0);
	write(data_channel(2), 0);
	return ERR_OK;
}

PROTECTED struct port port_8253 = {
	.type = PORT_8253,
	.request = request,
	.probe = probe,
	.setup = setup,
};

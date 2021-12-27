/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <print.h>

#include <drivers/interrupt.h>
#include <ports/8042.h>

#define TYPE_STANDARD_MOUSE 0x0000
#define TYPE_WHEEL_MOUSE 0x0003
#define TYPE_BUTTON_MOUSE 0x0004

#define RATE 0xf3
#define ENABLE 0xf4
#define DEFAULT_SETTINGS 0xf6

#define MOUSE(type)                                                                                \
	((type) == TYPE_STANDARD_MOUSE || (type) == TYPE_WHEEL_MOUSE || (type) == TYPE_BUTTON_MOUSE)

static u8 get(void)
{
	static u8 device = 0xff;
	if (device != 0xff)
		return device;

	u16 first = port_request(PORT_8042, PORT_8042_DEVICE_TYPE, 0);
	if (MOUSE(first)) {
		device = 0;
		return device;
	}

	u16 second = port_request(PORT_8042, PORT_8042_DEVICE_TYPE, 1);
	if (MOUSE(second)) {
		device = 1;
		return device;
	}

	log("Mouse device could not be found");
	return 0xff;
}

static void rate(u8 device, u8 rate)
{
	port_request(PORT_8042, PORT_8042_DEVICE_WRITE, device, RATE);
	port_request(PORT_8042, PORT_8042_DEVICE_WRITE, device, rate);
}

static err handler(void *data)
{
	UNUSED(data);

	u8 byte = 0;
	err read = port_read(PORT_8042, &byte, 0, 1);
	if (read != ERR_OK)
		return read;

	log("Mouse: %x", byte);

	return ERR_OK;
}

static err enable(void)
{
	err setup = port_setup(PORT_8042);
	if (setup != ERR_OK)
		return setup;

	u8 device = get();
	port_request(PORT_8042, PORT_8042_DEVICE_WRITE, device, DEFAULT_SETTINGS);
	port_request(PORT_8042, PORT_8042_DEVICE_WRITE, device, ENABLE);

	u16 type = port_request(PORT_8042, PORT_8042_DEVICE_TYPE, device);

	// Upgrade to wheel mouse if possible
	if (type != TYPE_WHEEL_MOUSE) {
		rate(device, 200);
		rate(device, 100);
		rate(device, 80);
		type = port_request(PORT_8042, PORT_8042_DEVICE_TYPE, device);
	}

	// Upgrade wheel mouse to button mouse if possible
	if (type == TYPE_WHEEL_MOUSE) {
		rate(device, 200);
		rate(device, 100);
		rate(device, 80);
		port_request(PORT_8042, PORT_8042_DEVICE_TYPE, device);
	}

	rate(device, 20);

	interrupt_register(12, handler);
	return ERR_OK;
}

static err disable(void)
{
	// TODO: 'disable mouse interrupt' request
	return -ERR_NOT_SUPPORTED;
}

static err probe(void)
{
	err works = port_probe(PORT_8042);
	if (works != ERR_OK)
		return works;

	u8 device = get();
	works = port_request(PORT_8042, PORT_8042_DEVICE_TEST, device);
	if (works != ERR_OK)
		return works;

	return ERR_OK;
}

PROTECTED struct device device_mouse = {
	.type = DEVICE_MOUSE,
	.enable = enable,
	.disable = disable,
	.probe = probe,
};

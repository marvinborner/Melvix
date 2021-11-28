/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <print.h>

#include <drivers/interrupt.h>
#include <ports/8042.h>

#define TYPE_TRANSLATION_KEYBOARD1 0xab41
#define TYPE_TRANSLATION_KEYBOARD2 0xabc1
#define TYPE_STANDARD_KEYBOARD 0xab83

#define KEYBOARD(type)                                                                             \
	((type) == TYPE_TRANSLATION_KEYBOARD1 || (type) == TYPE_TRANSLATION_KEYBOARD2 ||           \
	 (type) == TYPE_STANDARD_KEYBOARD)

static u8 get(void)
{
	static u8 device = 0xff;
	if (device != 0xff)
		return device;

	u16 first = port_request(PORT_8042, PORT_8042_DEVICE_TYPE, 0);
	if (KEYBOARD(first)) {
		device = 0;
		return device;
	}

	u16 second = port_request(PORT_8042, PORT_8042_DEVICE_TYPE, 1);
	if (KEYBOARD(second)) {
		device = 1;
		return device;
	}

	log("Keyboard device could not be found");
	return 0xff;
}

static err handler(void *data)
{
	UNUSED(data);

	u8 scancode = 0;
	err read = port_read(PORT_8042, &scancode, 0, 1);
	if (read != ERR_OK)
		return read;

	log("%x", scancode);

	return ERR_OK;
}

static err enable(void)
{
	err setup = port_setup(PORT_8042);
	if (setup != ERR_OK)
		return setup;

	interrupt_register(1, handler);
	return ERR_OK;
}

static err disable(void)
{
	// TODO: 'disable keyboard interrupt' request
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

PROTECTED struct device device_keyboard = {
	.type = DEVICE_KEYBOARD,
	.enable = enable,
	.disable = disable,
	.probe = probe,
};

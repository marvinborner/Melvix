/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <print.h>

#include <drivers/interrupt.h>
#include <events/keyboard.h>
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
	TRY(port_read(PORT_8042, &scancode, 0, 1));

	struct keyboard_data handler_data = { 0 }; // TODO
	TRY(keyboard_handler(&handler_data));

	return ERR_OK;
}

static err enable(void)
{
	TRY(port_setup(PORT_8042));

	interrupt_register(1, handler);
	return ERR_OK;
}

static err disable(void)
{
	// TODO: 'disable keyboard interrupt' request

	interrupt_remove(1);
	return ERR_OK;
}

static err probe(void)
{
	TRY(port_probe(PORT_8042));

	u8 device = get();
	TRY(port_request(PORT_8042, PORT_8042_DEVICE_TEST, device));

	return ERR_OK;
}

PROTECTED struct device device_keyboard = {
	.type = DEVICE_KEYBOARD,
	.enable = enable,
	.disable = disable,
	.probe = probe,
};

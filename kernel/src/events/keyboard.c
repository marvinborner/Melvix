/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <print.h>

#include <events/keyboard.h>

err keyboard_handler(struct keyboard_data *data)
{
	log("Keyboard: '%c'", KEY_TO_CHAR(data->key));
	return ERR_OK;
}

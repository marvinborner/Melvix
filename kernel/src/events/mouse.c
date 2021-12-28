/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <print.h>

#include <events/mouse.h>

err mouse_handler(struct mouse_data *data)
{
	log("Mouse: %dx%d", data->x, data->y);
	return ERR_OK;
}

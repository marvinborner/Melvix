/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <drivers/install.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <drivers/network.h>
#include <drivers/serial.h>
#include <drivers/video.h>

TEMPORARY void drivers_install(void)
{
	dev_add(&device_serial);
	/* dev_add(&device_video); */
	/* dev_add(&device_network); */
	/* dev_add(&device_keyboard); */
	/* dev_add(&device_mouse); */
}

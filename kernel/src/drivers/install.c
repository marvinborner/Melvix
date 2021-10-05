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
	dev_add(&serial_device);
	/* dev_add(&video_device); */
	/* dev_add(&network_device); */
	/* dev_add(&keyboard_device); */
	/* dev_add(&mouse_device); */
}

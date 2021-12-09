/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <drivers/install.h>
#include <drivers/interrupt.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <drivers/network.h>
#include <drivers/serial.h>
#include <drivers/video.h>

TEMPORARY void drivers_install(void)
{
	device_add(&device_serial);
	device_add(&device_interrupt);
	device_add(&device_keyboard);
	device_add(&device_mouse);
	/* device_add(&device_video); */
	/* device_add(&device_network); */
	__asm__ volatile("sti");
}

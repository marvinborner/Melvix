/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#ifndef SYS_H
#define SYS_H

#include <def.h>
#include <err.h>

/**
 * Device management
 */

enum device_type {
	DEVICE_INVALID,
	DEVICE_SERIAL,
	DEVICE_TIMER,
	DEVICE_INTERRUPT,
	DEVICE_FRAMEBUFFER,
	DEVICE_NETWORK,
	DEVICE_KEYBOARD,
	DEVICE_MOUSE,
	DEVICE_MAX,
};

NONNULL err device_read(enum device_type type, void *buf, u32 offset, u32 count);
NONNULL err device_write(enum device_type type, const void *buf, u32 offset, u32 count);
NONNULL err device_request(enum device_type type, u32 request, ...);

/**
 * Interrupt management
 */

typedef err (*interrupt_t)(void *data);

NONNULL err interrupt_call(u32 interrupt, void *data);
err interrupt_remove(u32 interrupt);
NONNULL err interrupt_register(u32 interrupt, interrupt_t handler);

/**
 * Port management
 */

enum port_type {
	PORT_8042,
	PORT_8250,
	PORT_8253,
	PORT_8259,
	PORT_MAX,
};

NONNULL err port_read(enum port_type type, void *buf, u32 offset, u32 count);
NONNULL err port_write(enum port_type type, const void *buf, u32 offset, u32 count);
NONNULL err port_request(enum port_type type, u32 request, ...);
err port_probe(enum port_type type);
err port_setup(enum port_type type);

/**
 * Timer management
 */

typedef void (*timer_callback_t)(u32 time);

err timer_execute(u32 time);
NONNULL err timer_interval(u32 interval, timer_callback_t callback);
NONNULL err timer_timeout(u32 timeout, timer_callback_t callback);

#endif

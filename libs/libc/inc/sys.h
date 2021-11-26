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

enum dev_type {
	DEV_INVALID,
	DEV_LOGGER,
	DEV_FRAMEBUFFER,
	DEV_NETWORK,
	DEV_KEYBOARD,
	DEV_MOUSE,
	DEV_MAX,
};

NONNULL err dev_read(enum dev_type type, void *buf, u32 offset, u32 count);
NONNULL err dev_write(enum dev_type type, const void *buf, u32 offset, u32 count);
NONNULL err dev_request(enum dev_type type, u32 request, ...);

/**
 * Port management
 */

enum port_type {
	PORT_8042,
	PORT_8250,
	PORT_MAX,
};

NONNULL err port_read(enum port_type type, void *buf, u32 offset, u32 count);
NONNULL err port_write(enum port_type type, const void *buf, u32 offset, u32 count);
NONNULL err port_request(enum port_type type, u32 request, ...);
NONNULL err port_probe(enum port_type type);
NONNULL err port_setup(enum port_type type);

#endif

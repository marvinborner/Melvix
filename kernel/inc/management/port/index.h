/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#ifndef MANAGEMENT_PORT_INDEX_H
#define MANAGEMENT_PORT_INDEX_H

#include <arg.h>
#include <sys.h>

typedef err (*port_read_t)(void *buf, u32 offset, u32 count);
typedef err (*port_write_t)(const void *buf, u32 offset, u32 count);
typedef err (*port_request_t)(u32 request, va_list);
typedef err (*port_probe_t)(void);
typedef err (*port_setup_t)(void);

struct port {
	enum port_type type;
	port_read_t read;
	port_write_t write;
	port_request_t request;
	port_probe_t probe;
	port_setup_t setup;
};

NONNULL void port_add(struct port *port);
struct port *port_get(enum port_type type);

#endif

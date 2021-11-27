/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <management/port/index.h>

err port_read(enum port_type type, void *buf, u32 offset, u32 count)
{
	struct port *port = port_get(type);
	if (!port)
		return -ERR_INVALID_ARGUMENTS;
	if (!port->read)
		return -ERR_NOT_SUPPORTED;
	return port->read(buf, offset, count);
}

err port_write(enum port_type type, const void *buf, u32 offset, u32 count)
{
	struct port *port = port_get(type);
	if (!port)
		return -ERR_INVALID_ARGUMENTS;
	if (!port->write)
		return -ERR_NOT_SUPPORTED;
	return port->write(buf, offset, count);
}

err port_request(enum port_type type, u32 request, ...)
{
	struct port *port = port_get(type);
	if (!port)
		return -ERR_INVALID_ARGUMENTS;
	if (!port->request)
		return -ERR_NOT_SUPPORTED;

	va_list ap;
	va_start(ap, request);
	int result = port->request(request, ap);
	va_end(ap);

	return result;
}

err port_probe(enum port_type type)
{
	struct port *port = port_get(type);
	if (!port)
		return -ERR_INVALID_ARGUMENTS;
	if (!port->probe)
		return -ERR_NOT_SUPPORTED;
	return port->probe();
}

err port_setup(enum port_type type)
{
	struct port *port = port_get(type);
	if (!port)
		return -ERR_INVALID_ARGUMENTS;
	if (!port->setup)
		return -ERR_NOT_SUPPORTED;
	return port->setup();
}

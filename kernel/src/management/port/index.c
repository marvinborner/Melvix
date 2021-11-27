/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <management/port/index.h>
#include <ports/8042.h>
#include <ports/8250.h>
#include <ports/8259.h>

PROTECTED static struct port *ports[PORT_MAX] = {
	&port_8042,
	&port_8250,
	&port_8259,
};

struct port *port_get(enum port_type type)
{
	if (type >= PORT_MAX || !ports[type] || ports[type]->type != type)
		return NULL;
	return ports[type];
}

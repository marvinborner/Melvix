/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <sys.h>

#include <management/interrupt/index.h>

err interrupt_call(u32 interrupt, void *data)
{
	interrupt_t handler = interrupt_get(interrupt);
	if (!handler)
		return -ERR_NOT_FOUND;
	return handler(data);
}

TEMPORARY err interrupt_register(u32 interrupt, interrupt_t handler)
{
	interrupt_add(interrupt, handler);
	return ERR_OK;
}

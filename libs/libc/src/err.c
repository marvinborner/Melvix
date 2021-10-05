/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <err.h>

static u32 error = 0;

u32 *__errno(void)
{
	return &error;
}

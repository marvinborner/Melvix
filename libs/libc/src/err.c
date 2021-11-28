/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <err.h>

static u32 error = 0;

const char *format_error(err code)
{
	if ((s8)code < 0)
		code = -(s8)code;

	switch (code) {
	case ERR_OK:
		return "OK";
	case ERR_NOT_FOUND:
		return "Not found";
	case ERR_NOT_SUPPORTED:
		return "Not supported";
	case ERR_INVALID_ARGUMENTS:
		return "Invalid arguments";
	case ERR_HARDWARE:
		return "Hardware failure";
	default:
		return "fuck";
	}
}

u32 *__errno(void)
{
	return &error;
}

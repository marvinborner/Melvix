/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <arg.h>
#include <format.h>
#include <print.h>
#include <sys.h>

NONNULL static u32 vlog(const char *fmt, va_list ap)
{
	char buf[1024] = { 0 };
	u32 len = format(buf, sizeof(buf), fmt, ap);
	dev_write(DEV_LOGGER, buf, 0, len);
	dev_write(DEV_LOGGER, "\n", 0, 1);
	// TODO: Write to device
	return len;
}

u32 log(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	u32 len = vlog(fmt, ap);
	va_end(ap);
	return len;
}

void panic(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vlog(fmt, ap);
	va_end(ap);

	// TODO: Actually panic
	while (1)
		;
}

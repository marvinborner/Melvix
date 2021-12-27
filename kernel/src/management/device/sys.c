/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <management/device/index.h>

err device_read(enum device_type type, void *buf, u32 offset, u32 count)
{
	struct device *device = device_get(type);
	if (!device)
		return -ERR_INVALID_ARGUMENTS;
	if (!device->read)
		return -ERR_NOT_SUPPORTED;
	return device->read(buf, offset, count);
}

err device_write(enum device_type type, const void *buf, u32 offset, u32 count)
{
	struct device *device = device_get(type);
	if (!device)
		return -ERR_INVALID_ARGUMENTS;
	if (!device->write)
		return -ERR_NOT_SUPPORTED;
	return device->write(buf, offset, count);
}

err device_request(enum device_type type, u32 request, ...)
{
	struct device *device = device_get(type);
	if (!device)
		return -ERR_INVALID_ARGUMENTS;
	if (!device->request)
		return -ERR_NOT_SUPPORTED;

	va_list ap;
	va_start(ap, request);
	err result = device->request(request, ap);
	va_end(ap);

	return result;
}

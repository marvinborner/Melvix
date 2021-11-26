/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <management/dev/index.h>
#include <management/dev/sys.h>

err dev_read(enum dev_type type, void *buf, u32 offset, u32 count)
{
	struct dev *dev = dev_get(type);
	if (!dev)
		return -ERR_INVALID_ARGUMENTS;
	if (!dev->read)
		return -ERR_NOT_SUPPORTED;
	return dev->read(buf, offset, count);
}

err dev_write(enum dev_type type, const void *buf, u32 offset, u32 count)
{
	struct dev *dev = dev_get(type);
	if (!dev)
		return -ERR_INVALID_ARGUMENTS;
	if (!dev->write)
		return -ERR_NOT_SUPPORTED;
	return dev->write(buf, offset, count);
}

err dev_request(enum dev_type type, u32 request, ...)
{
	struct dev *dev = dev_get(type);
	if (!dev)
		return -ERR_INVALID_ARGUMENTS;
	if (!dev->request)
		return -ERR_NOT_SUPPORTED;

	va_list ap;
	va_start(ap, request);
	int result = dev->request(request, ap);
	va_end(ap);

	return result;
}

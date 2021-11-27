/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#ifndef MANAGEMENT_DEVICE_INDEX_H
#define MANAGEMENT_DEVICE_INDEX_H

#include <arg.h>
#include <sys.h>

typedef err (*device_read_t)(void *buf, u32 offset, u32 count);
typedef err (*device_write_t)(const void *buf, u32 offset, u32 count);
typedef err (*device_request_t)(u32 request, va_list);
typedef err (*device_probe_t)(void);
typedef err (*device_enable_t)(void);
typedef err (*device_disable_t)(void);

struct device {
	enum device_type type;
	device_read_t read;
	device_write_t write;
	device_request_t request;
	device_probe_t probe;
	device_enable_t enable;
	device_disable_t disable;
};

NONNULL void device_add(struct device *device);
struct device *device_get(enum device_type type);

#endif

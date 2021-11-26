/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#ifndef MANAGEMENT_DEV_INDEX_H
#define MANAGEMENT_DEV_INDEX_H

#include <arg.h>
#include <sys.h>

typedef err (*dev_read_t)(void *buf, u32 offset, u32 count);
typedef err (*dev_write_t)(const void *buf, u32 offset, u32 count);
typedef err (*dev_request_t)(u32 request, va_list);
typedef err (*dev_probe_t)(void);
typedef err (*dev_enable_t)(void);
typedef err (*dev_disable_t)(void);

struct dev {
	enum dev_type type;
	dev_read_t read;
	dev_write_t write;
	dev_request_t request;
	dev_probe_t probe;
	dev_enable_t enable;
	dev_disable_t disable;
};

NONNULL void dev_add(struct dev *dev);
struct dev *dev_get(enum dev_type type);

#endif

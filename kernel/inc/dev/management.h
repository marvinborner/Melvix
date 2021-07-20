// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef DEV_MANAGEMENT_H
#define DEV_MANAGEMENT_H

#include <stdarg.h>
#include <stddef.h>
#include <sys/dev.h>
#include <sys/types.h>

#include <kernel.h>

typedef ssize_t (*dev_read_t)(void *, off_t, size_t);
typedef ssize_t (*dev_write_t)(const void *, off_t, size_t);
typedef int (*dev_request_t)(dev_req_t, va_list);
typedef void (*dev_enable_t)(void);
typedef void (*dev_disable_t)(void);

struct dev {
	u8 exists : 1;
	dev_t type;
	dev_read_t read;
	dev_write_t write;
	dev_request_t request;
	dev_enable_t enable;
	dev_disable_t disable;
};

void dev_add(struct dev *dev) NONNULL;
struct dev *dev_get(dev_t type);

#endif

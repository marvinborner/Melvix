// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef DEV_MANAGEMENT_H
#define DEV_MANAGEMENT_H

#include <stdarg.h>
#include <stddef.h>
#include <sys/dev.h>
#include <sys/types.h>

#include <kernel.h>

typedef int (*dev_read_t)(void *, off_t, size_t);
typedef int (*dev_request_t)(dev_req_t, va_list);

struct dev {
	u8 exists : 1;
	dev_t type;
	dev_read_t read;
	dev_request_t request;
};

void dev_add(struct dev *dev) NONNULL;
struct dev *dev_get(dev_t type);

#endif

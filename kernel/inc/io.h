// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef DEV_H
#define DEV_H

#include <def.h>
#include <sys.h>

struct io_dev {
	const char *name;
	res (*read)(void *buf, u32 offset, u32 count) NONNULL;
	res (*write)(void *buf, u32 offset, u32 count) NONNULL;
	res (*control)(u32 request, void *arg1, void *arg2, void *arg3);
};

void io_install(void);

res io_control(enum io_type io, u32 request, void *arg1, void *arg2, void *arg3);
res io_write(enum io_type io, void *buf, u32 offset, u32 count);
res io_read(enum io_type io, void *buf, u32 offset, u32 count);
res io_poll(u32 *devs);

#endif

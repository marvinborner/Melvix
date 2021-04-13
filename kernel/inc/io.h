// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef DEV_H
#define DEV_H

#include <def.h>
#include <sys.h>

enum io_type {
	IO_MIN,
	IO_FRAMEBUFFER,
	IO_NETWORK,
	IO_KEYBOARD,
	IO_MOUSE,
	IO_BUS,
	IO_MAX,
};

struct io_dev {
	const char *name;
};

void io_install(void);

res io_control();
res io_write(enum io_type io, void *buf, u32 offset, u32 count);
res io_read(enum io_type io, void *buf, u32 offset, u32 count);
res io_poll();

#endif

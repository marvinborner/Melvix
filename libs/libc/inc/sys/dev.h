// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef SYS_DEVICE_H
#define SYS_DEVICE_H

typedef enum {
	DEV_INVALID,
	DEV_LOGGER,
	DEV_KEYBOARD,
	DEV_MOUSE,
	DEV_FRAMEBUFFER,
	DEV_MAX,
} dev_t;

typedef unsigned long dev_req_t;

#endif

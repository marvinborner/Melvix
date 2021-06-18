// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef dev_H
#define dev_H

#include <def.h>
#include <drivers/int.h>
#include <proc.h>
#include <sys.h>

struct dev_dev {
	res (*read)(void *buf, u32 offset, u32 count) NONNULL;
	res (*write)(const void *buf, u32 offset, u32 count) NONNULL;
	res (*control)(u32 request, void *arg1, void *arg2, void *arg3);
	res (*ready)(void);
};

void dev_install(void);
void dev_add(enum dev_type type, struct dev_dev *dev) NONNULL;

// No NONNULL on syscalls
res dev_control(enum dev_type type, u32 request, void *arg1, void *arg2, void *arg3);
res dev_write(enum dev_type type, const void *buf, u32 offset, u32 count);
res dev_read(enum dev_type type, void *buf, u32 offset, u32 count);
res dev_poll(u32 *devs);
res dev_ready(enum dev_type type);

void dev_block(enum dev_type type, struct proc *proc) NONNULL;
void dev_unblock(enum dev_type type);
void dev_unblock_pid(u32 pid);

#endif

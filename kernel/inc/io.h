// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef DEV_H
#define DEV_H

#include <boot.h>
#include <def.h>
#include <interrupts.h>
#include <proc.h>
#include <sys.h>

struct io_dev {
	res (*read)(void *buf, u32 offset, u32 count) NONNULL;
	res (*write)(void *buf, u32 offset, u32 count) NONNULL;
	res (*control)(u32 request, void *arg1, void *arg2, void *arg3);
	res (*ready)(void);
};

void io_install(struct boot_info *boot);
void io_add(enum io_type io, struct io_dev *dev) NONNULL;

res io_control(enum io_type io, u32 request, void *arg1, void *arg2, void *arg3);
res io_write(enum io_type io, void *buf, u32 offset, u32 count);
res io_read(enum io_type io, void *buf, u32 offset, u32 count);
res io_poll(u32 *devs) NONNULL;

void io_block(enum io_type io, struct proc *proc, struct regs *r) NONNULL;
void io_unblock(enum io_type io);

#endif

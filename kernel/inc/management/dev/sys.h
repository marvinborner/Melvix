// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef MANAGEMENT_DEV_SYS_H
#define MANAGEMENT_DEV_SYS_H

#include <def.h>
#include <sys.h>

#include <management/dev/index.h>

res sys_dev_write(enum dev_type type, const void *buf, u32 offset, u32 count);
res sys_dev_read(enum dev_type type, void *buf, u32 offset, u32 count);
res sys_dev_poll(struct dev_poll *fds, u32 nfds, int timeout);
res sys_dev_request(enum dev_type type, u32 request, ...);

#endif

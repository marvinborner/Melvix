// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef DEV_SYS_H
#define DEV_SYS_H

#include <poll.h>
#include <stddef.h>
#include <sys/dev.h>
#include <sys/types.h>

ssize_t sys_dev_read(dev_t type, void *buf, off_t offset, size_t count);
ssize_t sys_dev_write(dev_t type, const void *buf, off_t offset, size_t count);
int sys_dev_poll(struct pollfd *fds, nfds_t nfds, int timeout);
int sys_dev_request(dev_t type, dev_req_t request, ...);

#endif

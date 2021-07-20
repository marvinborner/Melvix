// MIT License, Copyright(c) 2021 Marvin Borner

#ifndef UNISTD_H
#define UNISTD_H

#include <poll.h>
#include <stddef.h>
#include <sys/dev.h>
#include <sys/stat.h>
#include <sys/types.h>

ssize_t fs_read(const char *pathname, void *buf, off_t offset, size_t count);
ssize_t fs_write(const char *pathname, const void *buf, off_t offset, size_t count);
int fs_stat(const char *pathname, struct stat *statbuf);
int fs_create(const char *pathname);

ssize_t dev_read(dev_t type, void *buf, off_t offset, size_t count);
ssize_t dev_write(dev_t type, const void *buf, off_t offset, size_t count);
int dev_poll(struct pollfd *fds, nfds_t nfds, int timeout);
int dev_request(dev_t type, dev_req_t request, ...);

#endif

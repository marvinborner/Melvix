// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef FS_VIRTUAL_H
#define FS_VIRTUAL_H

#include <poll.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <kernel.h>

ssize_t sys_read(int fd, void *buf, size_t count);
ssize_t sys_write(int fd, const void *buf, size_t count);
int sys_open(const char *pathname, int flags, mode_t mode);
int sys_close(int fd);
int sys_stat(int fd, struct stat *statbuf);
int sys_poll(struct pollfd *fds, nfds_t nfds, int timeout);
off_t sys_seek(int fd, off_t offset, int whence);
int sys_ioctl(int fd, unsigned long request, ...);
int sys_access(const char *pathname, int mode);
int sys_create(const char *pathname, mode_t mode);
int sys_mknode(const char *pathname, mode_t mode, dev_t dev);

#endif

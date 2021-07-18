// MIT License, Copyright (c) 2021 Marvin Borner

#include <fs/virtual.h>

/**
 * Syscalls
 */

ssize_t sys_read(int fd, void *buf, size_t count)
{
	return 0;
}

ssize_t sys_write(int fd, const void *buf, size_t count)
{
	return 0;
}

int sys_open(const char *pathname, int flags, mode_t mode)
{
	return 0;
}

int sys_close(int fd)
{
	return 0;
}

int sys_stat(int fd, struct stat *statbuf)
{
	return 0;
}

int sys_poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
	return 0;
}

off_t sys_seek(int fd, off_t offset, int whence)
{
	return 0;
}

int sys_ioctl(int fd, unsigned long request, ...)
{
	return 0;
}

int sys_access(const char *pathname, int mode)
{
	return 0;
}

int sys_create(const char *pathname, mode_t mode)
{
	return 0;
}

int sys_mknode(const char *pathname, mode_t mode, dev_t dev)
{
	return 0;
}

// MIT License, Copyright (c) 2021 Marvin Borner

#include <fs/sys.h>

ssize_t sys_fs_read(const char *path, void *buf, off_t offset, size_t count)
{
	return 0;
}

ssize_t sys_fs_write(const char *path, const void *buf, off_t offset, size_t count)
{
	return 0;
}

int sys_fs_stat(const char *path, struct stat *statbuf)
{
	return 0;
}

int sys_fs_create(const char *path)
{
	return 0;
}

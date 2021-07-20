// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef FS_SYS_H
#define FS_SYS_H

#include <stddef.h>
#include <sys/stat.h>
#include <sys/types.h>

ssize_t sys_fs_read(const char *pathname, void *buf, off_t offset, size_t count);
ssize_t sys_fs_write(const char *pathname, const void *buf, off_t offset, size_t count);
int sys_fs_stat(const char *pathname, struct stat *statbuf);
int sys_fs_create(const char *pathname);

#endif

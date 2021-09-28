// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef FS_SYS_H
#define FS_SYS_H

#include <def.h>
#include <errno.h>
#include <sys.h>

// No NONNULL on syscalls
res sys_vfs_read(const char *path, void *buf, u32 offset, u32 count);
res sys_vfs_write(const char *path, const void *buf, u32 offset, u32 count);
res sys_vfs_stat(const char *path, struct stat *buf);

#endif

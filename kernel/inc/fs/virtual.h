// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef FS_VIRTUAL_H
#define FS_VIRTUAL_H

#include <def.h>
#include <errno.h>
#include <sys.h>

/**
 * Device
 */

enum vfs_dev_type { DEV_BLOCK, DEV_CHAR };

struct vfs_dev {
	u32 id;
	char name[8];
	enum vfs_dev_type type;
	struct vfs *vfs;
	void *data;
	res (*read)(void *buf, u32 offset, u32 count, struct vfs_dev *dev) NONNULL;
	res (*write)(const void *buf, u32 offset, u32 count, struct vfs_dev *dev) NONNULL;
};

/**
 * VFS
 */

enum vfs_type { VFS_TMPFS, VFS_PROCFS, VFS_EXT2 };
enum vfs_perm { VFS_EXEC, VFS_WRITE, VFS_READ };

struct vfs {
	enum vfs_type type;
	int flags;
	void *data;
	res (*read)(const char *path, void *buf, u32 offset, u32 count,
		    struct vfs_dev *dev) NONNULL;
	res (*write)(const char *path, const void *buf, u32 offset, u32 count,
		     struct vfs_dev *dev) NONNULL;
	res (*stat)(const char *path, struct stat *buf, struct vfs_dev *dev) NONNULL;
	res (*block)(const char *path, u32 func_ptr, struct vfs_dev *dev) NONNULL;
	res (*perm)(const char *path, enum vfs_perm perm, struct vfs_dev *dev) NONNULL;
};

struct mount_info {
	const char *path;
	struct vfs_dev *dev;
};

void vfs_install(void);

char *vfs_normalize_path(const char *path);

u8 vfs_mounted(struct vfs_dev *dev, const char *path) NONNULL;
u8 vfs_mount(struct vfs_dev *dev, const char *path) NONNULL;
struct mount_info *vfs_find_mount_info(const char *path) NONNULL;

struct vfs_dev *vfs_find_dev(const char *path) NONNULL;
void vfs_add_dev(struct vfs_dev *dev) NONNULL;

void vfs_load(struct vfs_dev *dev) NONNULL;

struct vfs_dev *device_get_by_name(const char *name) NONNULL;
struct vfs_dev *device_get_by_id(u32 id) NONNULL;

#endif

// MIT License, Copyright (c) 2021 Marvin Borner

#include <list.h>
#include <mem.h>
#include <str.h>

#include <fs/sys.h>
#include <fs/virtual.h>
#include <memory/range.h>

res sys_vfs_read(const char *path, void *buf, u32 offset, u32 count)
{
	if (!memory_readable(path))
		return -EFAULT;

	if (!memory_writable_range(memory_range(buf, count)))
		return -EFAULT;

	struct mount_info *m = vfs_find_mount_info(path);
	if (!m || !m->dev || !m->dev->vfs)
		return -ENOENT;

	if (!m->dev->vfs->read || !m->dev->vfs->perm)
		return -EINVAL;

	u32 len = strlen(m->path);
	if (len > 1)
		path += len;

	res perm = m->dev->vfs->perm(path, VFS_READ, m->dev);
	if (perm != EOK)
		return perm;

	if (!count)
		return EOK;

	return m->dev->vfs->read(path, buf, offset, count, m->dev);
}

res sys_vfs_write(const char *path, const void *buf, u32 offset, u32 count)
{
	if (!memory_readable(path))
		return -EFAULT;

	if (!memory_readable_range(memory_range(buf, count)))
		return -EFAULT;

	struct mount_info *m = vfs_find_mount_info(path);
	if (!m || !m->dev || !m->dev->vfs)
		return -ENOENT;

	if (!m->dev->vfs->write || !m->dev->vfs->perm)
		return -EINVAL;

	u32 len = strlen(m->path);
	if (len > 1)
		path += len;

	res perm = m->dev->vfs->perm(path, VFS_WRITE, m->dev);
	if (perm != EOK)
		return perm;

	if (!count)
		return EOK;

	return m->dev->vfs->write(path, buf, offset, count, m->dev);
}

res sys_vfs_stat(const char *path, struct stat *buf)
{
	if (!memory_readable(path))
		return -EFAULT;

	if (!memory_writable_range(memory_range(buf, sizeof(*buf))))
		return -EFAULT;

	struct mount_info *m = vfs_find_mount_info(path);
	if (!m || !m->dev || !m->dev->vfs)
		return -ENOENT;

	if (!m->dev->vfs->stat || !m->dev->vfs->perm)
		return -EINVAL;

	u32 len = strlen(m->path);
	if (len > 1)
		path += len;

	res perm = m->dev->vfs->perm(path, VFS_READ, m->dev);
	if (perm != EOK)
		return perm;

	return m->dev->vfs->stat(path, buf, m->dev);
}

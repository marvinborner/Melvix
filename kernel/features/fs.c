// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <cpu.h>
#include <crypto.h>
#include <def.h>
#include <errno.h>
#include <fs.h>
#include <ide.h>
#include <mbr.h>
#include <mem.h>
#include <mm.h>
#include <print.h>
#include <rand.h>
#include <str.h>

/**
 * VFS
 */

PROTECTED static struct list *mount_points = NULL;

static char *vfs_normalize_path(const char *path)
{
	char *fixed = strdup(path);
	int len = strlen(fixed);
	if (fixed[len - 1] == '/' && len != 1)
		fixed[len - 1] = '\0';
	return fixed;
}

u8 vfs_mounted(struct vfs_dev *dev, const char *path)
{
	struct node *iterator = mount_points->head;
	while (iterator) {
		if (((struct mount_info *)iterator->data)->dev->id == dev->id ||
		    !strcmp(((struct mount_info *)iterator->data)->path, path))
			return 1;
		iterator = iterator->next;
	}
	return 0;
}

// TODO: Reduce allocations in VFS find
static struct mount_info *vfs_recursive_find(char *path)
{
	struct node *iterator = mount_points->head;
	char *fixed = vfs_normalize_path(path);
	free(path); // Due to recursiveness

	while (iterator) {
		struct mount_info *m = iterator->data;
		if (!strcmp(m->path, fixed)) {
			free(fixed);
			return m;
		}
		iterator = iterator->next;
	}

	if (strlen(fixed) == 1) {
		free(fixed);
		return NULL;
	}

	*(strrchr(fixed, '/') + 1) = '\0';
	return vfs_recursive_find(fixed);
}

static struct mount_info *vfs_find_mount_info(const char *path)
{
	stac();
	if (path[0] != '/') {
		clac();
		return NULL;
	}
	clac();
	struct mount_info *ret = vfs_recursive_find(strdup_user(path));
	return ret;
}

CLEAR void vfs_add_dev(struct vfs_dev *dev)
{
	dev->id = rand() + 1;
}

struct vfs_dev *vfs_find_dev(const char *path)
{
	stac();
	if (path[0] != '/') {
		clac();
		return NULL;
	}
	clac();
	struct mount_info *m = vfs_find_mount_info(path);
	return m && m->dev ? m->dev : NULL;
}

res vfs_mount(struct vfs_dev *dev, const char *path)
{
	if (!memory_readable(path))
		return -EFAULT;

	if (!memory_readable(dev) || !dev->id)
		return -EFAULT;

	if (vfs_mounted(dev, path))
		return -EBUSY;

	char *fixed = vfs_normalize_path(path);

	struct mount_info *m = malloc(sizeof(*m));
	m->path = fixed;
	m->dev = dev;
	list_add(mount_points, m);

	return EOK;
}

res vfs_read(const char *path, void *buf, u32 offset, u32 count)
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

	if (m->dev->vfs->perm(path, VFS_READ, m->dev) != EOK && !proc_super())
		return -EACCES;

	if (!count)
		return EOK;

	return m->dev->vfs->read(path, buf, offset, count, m->dev);
}

res vfs_write(const char *path, const void *buf, u32 offset, u32 count)
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

	if (m->dev->vfs->perm(path, VFS_WRITE, m->dev) != EOK && !proc_super())
		return -EACCES;

	if (!count)
		return EOK;

	return m->dev->vfs->write(path, buf, offset, count, m->dev);
}

res vfs_stat(const char *path, struct stat *buf)
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

	if (m->dev->vfs->perm(path, VFS_READ, m->dev) != EOK && !proc_super())
		return -EACCES;

	return m->dev->vfs->stat(path, buf, m->dev);
}

CLEAR void vfs_load(struct vfs_dev *dev)
{
	if (!mbr_load(dev)) {
		assert(ext2_load(dev));
		// TODO: Add GPT support
	}
}

CLEAR void vfs_install(void)
{
	mount_points = list_new();
}

/**
 * EXT2
 */

INLINE static void ext2_buffer_read(u32 block, void *buf, struct vfs_dev *dev)
{
	dev->read(buf, block * SECTOR_COUNT, SECTOR_COUNT, dev);
}

static void ext2_get_superblock(struct ext2_superblock *buf, struct vfs_dev *dev)
{
	u8 data[BLOCK_SIZE] = { 0 };
	ext2_buffer_read(EXT2_SUPER, data, dev);
	memcpy(buf, data, sizeof(*buf));

	assert(buf->magic == EXT2_MAGIC);
}

static struct ext2_inode *ext2_get_inode(u32 i, struct ext2_inode *in_buf, struct vfs_dev *dev)
{
	struct ext2_superblock sb = { 0 };
	ext2_get_superblock(&sb, dev);

	u8 data[BLOCK_SIZE] = { 0 };
	ext2_buffer_read(EXT2_SUPER + 1, data, dev);
	struct ext2_bgd *bgd = (struct ext2_bgd *)data;

	u32 block_group = (i - 1) / sb.inodes_per_group;
	u32 index = (i - 1) % sb.inodes_per_group;
	u32 block = (index * EXT2_INODE_SIZE) / BLOCK_SIZE;
	bgd += block_group;

	u8 buf[BLOCK_SIZE] = { 0 };
	ext2_buffer_read(bgd->inode_table + block, buf, dev);
	struct ext2_inode *in =
		(struct ext2_inode *)((u32)buf +
				      (index % (BLOCK_SIZE / EXT2_INODE_SIZE)) * EXT2_INODE_SIZE);

	memcpy(in_buf, in, sizeof(*in_buf));

	return in_buf;
}

struct indirect_cache {
	u32 block;
	u8 data[BLOCK_SIZE];
};
static struct list *indirect_cache = NULL;
static u32 ext2_read_indirect(u32 indirect, u32 block_num, struct vfs_dev *dev)
{
	void *data = NULL;
	if (indirect_cache) {
		struct node *iterator = indirect_cache->head;
		while (iterator) {
			struct indirect_cache *cache = iterator->data;
			if (cache->block == indirect)
				data = cache->data;
			iterator = iterator->next;
		}
	} else {
		indirect_cache = list_new();
	}

	if (!data) {
		u8 buf[BLOCK_SIZE] = { 0 };
		ext2_buffer_read(indirect, buf, dev);
		struct indirect_cache *cache = malloc(sizeof(*cache));
		cache->block = indirect;
		memcpy(cache->data, buf, BLOCK_SIZE);
		list_add(indirect_cache, cache);
		u32 ind = *(u32 *)((u32)buf + block_num * sizeof(u32));
		return ind;
	}

	u32 ind = *(u32 *)((u32)data + block_num * sizeof(u32));
	return ind;
}

static res ext2_read_inode(struct ext2_inode *in, void *buf, u32 offset, u32 count,
			   struct vfs_dev *dev)
{
	if (!in || !buf)
		return -EINVAL;

	if (in->size == 0)
		return EOK;

	u32 num_blocks = in->blocks / (BLOCK_SIZE / SECTOR_SIZE) + 1;

	if (!num_blocks)
		return -EINVAL;

	u32 first_block = offset / BLOCK_SIZE;
	u32 last_block = (offset + count) / BLOCK_SIZE;
	if (last_block >= num_blocks)
		last_block = num_blocks - 1;
	u32 first_block_offset = offset % BLOCK_SIZE;

	u32 remaining = MIN(count, in->size - offset);
	u32 copied = 0;

	u32 indirect, blocknum;

	// TODO: Support triply indirect pointers
	for (u32 i = first_block; i <= last_block; i++) {
		if (i < 12) {
			blocknum = in->block[i];
		} else if (i < BLOCK_COUNT + 12) {
			indirect = in->block[12];
			blocknum = ext2_read_indirect(indirect, i - 12, dev);
		} else {
			indirect = in->block[13];
			blocknum = ext2_read_indirect(indirect,
						      (i - (BLOCK_COUNT + 12)) / BLOCK_COUNT, dev);
			blocknum = ext2_read_indirect(blocknum,
						      (i - (BLOCK_COUNT + 12)) % BLOCK_COUNT, dev);
		}

		u8 data[BLOCK_SIZE] = { 0 };
		ext2_buffer_read(blocknum, data, dev);
		u32 block_offset = (i == first_block) ? first_block_offset : 0;
		u32 byte_count = MIN(BLOCK_SIZE - block_offset, remaining);

		memcpy_user((u8 *)buf + copied, data + block_offset, byte_count);

		copied += byte_count;
		remaining -= byte_count;

		/* printf("Loaded %d of %d\n", i + 1, last_block); */
	}

	if (indirect_cache) {
		struct node *iterator = indirect_cache->head;
		while (iterator) {
			struct indirect_cache *cache = iterator->data;
			free(cache);
			iterator = iterator->next;
		}
		list_destroy(indirect_cache);
		indirect_cache = NULL;
	}

	return copied;
}

static u32 ext2_find_inode(const char *name, u32 dir_inode, struct vfs_dev *dev)
{
	if ((signed)dir_inode <= 0)
		return (unsigned)-1;

	struct ext2_inode i = { 0 };
	ext2_get_inode(dir_inode, &i, dev);

	char *buf = zalloc(BLOCK_SIZE * i.blocks / 2);

	u8 data[BLOCK_SIZE] = { 0 };
	for (u32 q = 0; q < i.blocks / 2; q++) {
		ext2_buffer_read(i.block[q], data, dev);
		memcpy((u32 *)((u32)buf + q * BLOCK_SIZE), data, BLOCK_SIZE);
	}

	struct ext2_dirent *d = (struct ext2_dirent *)buf;

	u32 sum = 0;
	do {
		// Calculate the 4byte aligned size of each entry
		sum += d->total_len;
		if (strlen(name) == d->name_len &&
		    strncmp((void *)d->name, name, d->name_len) == 0) {
			free(buf);
			return d->inode_num;
		}
		d = (struct ext2_dirent *)((u32)d + d->total_len);

	} while (sum < (1024 * i.blocks / 2));
	free(buf);
	return (unsigned)-1;
}

static struct ext2_inode *ext2_find_inode_by_path(const char *path, struct ext2_inode *in_buf,
						  struct vfs_dev *dev)
{
	char *path_cp = strdup_user(path);
	char *init = path_cp; // For freeing

	if (path_cp[0] != '/') {
		free(init);
		return NULL;
	}

	path_cp++;
	u32 current_inode = EXT2_ROOT;

	while (1) {
		u32 i;
		for (i = 0; path_cp[i] != '/' && path_cp[i] != '\0'; i++)
			;

		if (path_cp[i] == '\0')
			break;

		path_cp[i] = '\0';
		current_inode = ext2_find_inode(path_cp, current_inode, dev);
		path_cp[i] = '/';

		if ((signed)current_inode <= 0) {
			free(init);
			return NULL;
		}

		path_cp += i + 1;
	}

	u32 inode = ext2_find_inode(path_cp, current_inode, dev);
	free(init);
	if ((signed)inode <= 0)
		return NULL;

	return ext2_get_inode(inode, in_buf, dev);
}

static res ext2_read(const char *path, void *buf, u32 offset, u32 count, struct vfs_dev *dev)
{
	struct ext2_inode in = { 0 };
	if (ext2_find_inode_by_path(path, &in, dev) == &in) {
		return ext2_read_inode(&in, buf, offset, count, dev);
	} else
		return -ENOENT;
}

static res ext2_stat(const char *path, struct stat *buf, struct vfs_dev *dev)
{
	struct ext2_inode in = { 0 };
	if (ext2_find_inode_by_path(path, &in, dev) != &in)
		return -ENOENT;

	stac();
	buf->dev_id = dev->id;
	buf->size = in.size;
	clac();

	return EOK;
}

static res ext2_perm(const char *path, enum vfs_perm perm, struct vfs_dev *dev)
{
	struct ext2_inode in = { 0 };
	if (ext2_find_inode_by_path(path, &in, dev) != &in)
		return -ENOENT;

	switch (perm) {
	case VFS_EXEC:
		return (in.mode & EXT2_PERM_UEXEC) != 0 ? EOK : -EACCES;
	case VFS_WRITE:
		return (in.mode & EXT2_PERM_UWRITE) != 0 ? EOK : -EACCES;
	case VFS_READ:
		return (in.mode & EXT2_PERM_UREAD) != 0 ? EOK : -EACCES;
	default:
		return -EINVAL;
	}
}

CLEAR u8 ext2_load(struct vfs_dev *dev)
{
	struct ext2_superblock sb = { 0 };
	ext2_get_superblock(&sb, dev);
	if (sb.magic != EXT2_MAGIC)
		return 0;

	struct vfs *vfs = zalloc(sizeof(*vfs));
	vfs->type = VFS_EXT2;
	vfs->read = ext2_read;
	vfs->stat = ext2_stat;
	vfs->perm = ext2_perm;
	dev->vfs = vfs;

	// Verify that '/' is unmounted
	struct mount_info *m = vfs_find_mount_info("/");
	if (m && m->dev)
		panic("Found multiple ext2 disks!\n");

	// TODO: Mount other ext2 disks somewhere else
	printf("Mounting disk %s to '/'\n", dev->name);
	vfs_mount(dev, "/");

	return 1;
}

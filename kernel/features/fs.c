// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <cpu.h>
#include <crypto.h>
#include <def.h>
#include <errno.h>
#include <fs.h>
#include <ide.h>
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

/*static const char *vfs_resolve_type(enum vfs_type type)
{
	switch (type) {
	case VFS_DEVFS:
		return "devfs";
	case VFS_TMPFS:
		return "tmpfs";
	case VFS_PROCFS:
		return "procfs";
	case VFS_EXT2:
		return "ext2";
	default:
		return "unknown";
	}
}

static void vfs_list_mounts()
{
	struct node *iterator = mount_points->head;
	while (iterator) {
		struct mount_info *m = iterator->data;
		printf("%s on %s type %s\n", m->dev->name, m->path,
		       vfs_resolve_type(m->dev->vfs->type));
		iterator = iterator->next;
	}
}*/

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

	if (!memory_writable(buf))
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

res vfs_write(const char *path, void *buf, u32 offset, u32 count)
{
	if (!memory_readable(path))
		return -EFAULT;

	if (!memory_writable(buf))
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

res vfs_ioctl(const char *path, u32 request, void *arg1, void *arg2, void *arg3)
{
	if (!memory_readable(path))
		return -EFAULT;

	struct mount_info *m = vfs_find_mount_info(path);
	if (!m || !m->dev || !m->dev->vfs)
		return -ENOENT;

	if (!m->dev->vfs->ioctl || !m->dev->vfs->perm)
		return -EINVAL;

	u32 len = strlen(m->path);
	if (len > 1)
		path += len;

	if (m->dev->vfs->perm(path, VFS_WRITE, m->dev) != EOK && !proc_super())
		return -EACCES;

	return m->dev->vfs->ioctl(path, request, arg1, arg2, arg3, m->dev);
}

res vfs_stat(const char *path, struct stat *buf)
{
	if (!memory_readable(path))
		return -EFAULT;

	if (!memory_writable(buf))
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

CLEAR void vfs_install(void)
{
	mount_points = list_new();
}

/**
 * EXT2
 */

// TODO: Remove malloc from ext2_buffer_read (attempt in #56cd63f199)
static void *ext2_buffer_read(u32 block, struct vfs_dev *dev)
{
	void *buf = zalloc(BLOCK_SIZE);
	dev->read(buf, block * SECTOR_COUNT, SECTOR_COUNT, dev);
	return buf;
}

static struct ext2_superblock *ext2_get_superblock(struct vfs_dev *dev)
{
	struct ext2_superblock *sb = ext2_buffer_read(EXT2_SUPER, dev);

	assert(sb->magic == EXT2_MAGIC);
	return sb;
}

static struct ext2_bgd *ext2_get_bgd(struct vfs_dev *dev)
{
	return ext2_buffer_read(EXT2_SUPER + 1, dev);
}

static struct ext2_inode *ext2_get_inode(u32 i, struct ext2_inode *in_buf, struct vfs_dev *dev)
{
	struct ext2_superblock *s = ext2_get_superblock(dev);
	assert(s);
	struct ext2_bgd *b = ext2_get_bgd(dev);
	assert(b);

	u32 block_group = (i - 1) / s->inodes_per_group;
	u32 index = (i - 1) % s->inodes_per_group;
	u32 block = (index * EXT2_INODE_SIZE) / BLOCK_SIZE;
	b += block_group;

	u32 *buf = ext2_buffer_read(b->inode_table + block, dev);
	struct ext2_inode *in =
		(struct ext2_inode *)((u32)buf +
				      (index % (BLOCK_SIZE / EXT2_INODE_SIZE)) * EXT2_INODE_SIZE);

	memcpy(in_buf, in, sizeof(*in_buf));
	free(buf);
	free(s);
	free(b - block_group);

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
		data = ext2_buffer_read(indirect, dev);
		struct indirect_cache *cache = malloc(sizeof(*cache));
		cache->block = indirect;
		memcpy(cache->data, data, BLOCK_SIZE);
		list_add(indirect_cache, cache);
		u32 ind = *(u32 *)((u32)data + block_num * sizeof(u32));
		free(data);
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

	u32 indirect = 0;
	u32 blocknum = 0;

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

		char *data = ext2_buffer_read(blocknum, dev);
		u32 block_offset = (i == first_block) ? first_block_offset : 0;
		u32 byte_count = MIN(BLOCK_SIZE - block_offset, remaining);

		memcpy_user((u8 *)buf + copied, data + block_offset, byte_count);

		copied += byte_count;
		remaining -= byte_count;

		free(data);
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

	char *buf = malloc(BLOCK_SIZE * i.blocks / 2);
	memset(buf, 0, BLOCK_SIZE * i.blocks / 2);

	for (u32 q = 0; q < i.blocks / 2; q++) {
		char *data = ext2_buffer_read(i.block[q], dev);
		memcpy((u32 *)((u32)buf + q * BLOCK_SIZE), data, BLOCK_SIZE);
		free(data);
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

	u32 i = 0;
	while (1) {
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

res ext2_read(const char *path, void *buf, u32 offset, u32 count, struct vfs_dev *dev)
{
	struct ext2_inode in = { 0 };
	if (ext2_find_inode_by_path(path, &in, dev) == &in) {
		return ext2_read_inode(&in, buf, offset, count, dev);
	} else
		return -ENOENT;
}

res ext2_stat(const char *path, struct stat *buf, struct vfs_dev *dev)
{
	struct ext2_inode in = { 0 };
	if (ext2_find_inode_by_path(path, &in, dev) != &in)
		return -ENOENT;

	//u32 num_blocks = in.blocks / (BLOCK_SIZE / SECTOR_SIZE);
	//u32 sz = BLOCK_SIZE * num_blocks;

	stac();
	buf->dev_id = dev->id;
	buf->size = in.size;
	clac();

	return EOK;
}

res ext2_perm(const char *path, enum vfs_perm perm, struct vfs_dev *dev)
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

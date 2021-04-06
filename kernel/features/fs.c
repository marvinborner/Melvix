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
#include <random.h>
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

u8 vfs_mounted(struct device *dev, const char *path)
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

struct device *vfs_find_dev(const char *path)
{
	stac();
	if (path[0] != '/') {
		clac();
		return NULL;
	}
	clac();
	struct mount_info *m = vfs_find_mount_info(path);
	if (m->dev->vfs->type == VFS_DEVFS) // TODO: ?
		return device_get_by_name(path + strlen(m->path) + 1);
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

res vfs_mount(struct device *dev, const char *path)
{
	if (!memory_valid(path))
		return -EFAULT;

	if (!memory_valid(dev) || !dev->id)
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
	if (!memory_valid(path))
		return -EFAULT;

	if (!memory_valid(buf))
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
	if (!memory_valid(path))
		return -EFAULT;

	if (!memory_valid(buf))
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
	if (!memory_valid(path))
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
	if (!memory_valid(path))
		return -EFAULT;

	if (!memory_valid(buf))
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

res vfs_block(const char *path, u32 func_ptr)
{
	if (!func_ptr || !memory_valid(path))
		return -EFAULT;

	struct mount_info *m = vfs_find_mount_info(path);
	if (!m || !m->dev || !m->dev->vfs)
		return -ENOENT;

	// Default block
	if (!m->dev->vfs->block) {
		proc_block(vfs_find_dev(path)->id, PROC_BLOCK_DEV, func_ptr);
		return EOK;
	}

	u32 len = strlen(m->path);
	if (len > 1)
		path += len;

	return m->dev->vfs->block(path, func_ptr, m->dev);
}

// TODO: Reduce stac clac?
// TODO: Fix page fault when called too often/fast
res vfs_poll(const char **files)
{
	if (!memory_valid(files))
		return -EFAULT;

	stac();
	for (const char **p = files; *p && memory_valid(*p) && **p; p++) {
		res ready = vfs_ready(*p);
		clac();
		if (ready == 1)
			return p - files;
		else if (ready < 0)
			return ready;
		stac();
	}
	clac();

	stac();
	for (const char **p = files; *p && memory_valid(*p) && **p; p++) {
		vfs_block(*p, (u32)vfs_poll);
		stac();
	}
	clac();

	return PROC_MAX_BLOCK_IDS + 1;
}

res vfs_ready(const char *path)
{
	if (!memory_valid(path))
		return -EFAULT;

	struct mount_info *m = vfs_find_mount_info(path);
	if (!m || !m->dev || !m->dev->vfs)
		return -ENOENT;

	if (!m->dev->vfs->ready)
		return -EINVAL;

	u32 len = strlen(m->path);
	if (len > 1)
		path += len;

	return m->dev->vfs->ready(path, m->dev);
}

CLEAR void vfs_install(void)
{
	mount_points = list_new();
}

/**
 * Device
 */

PROTECTED static struct list *devices = NULL;

CLEAR void device_add(struct device *dev)
{
	dev->id = rand() + 1;
	list_add(devices, dev);
}

struct device *device_get_by_id(u32 id)
{
	struct node *iterator = devices->head;
	while (iterator) {
		if (((struct device *)iterator->data)->id == id)
			return iterator->data;
		iterator = iterator->next;
	}
	return NULL;
}

struct device *device_get_by_name(const char *name)
{
	struct node *iterator = devices->head;
	while (iterator) {
		if (!strcmp_user(((struct device *)iterator->data)->name, name))
			return iterator->data;
		iterator = iterator->next;
	}
	return NULL;
}

static res devfs_read(const char *path, void *buf, u32 offset, u32 count, struct device *dev)
{
	struct device *target = device_get_by_name(path + 1);
	if (!target)
		return -ENOENT;
	if (!target->read)
		return -EINVAL;
	return target->read(buf, offset, count, dev);
}

static res devfs_ioctl(const char *path, u32 request, void *arg1, void *arg2, void *arg3,
		       struct device *dev)
{
	struct device *target = device_get_by_name(path + 1);
	if (!target)
		return -ENOENT;
	if (!target->ioctl)
		return -EINVAL;
	return target->ioctl(request, arg1, arg2, arg3, dev);
}

static res devfs_perm(const char *path, enum vfs_perm perm, struct device *dev)
{
	UNUSED(path);
	UNUSED(perm);
	UNUSED(dev);
	return EOK;
}

static res devfs_ready(const char *path, struct device *dev)
{
	UNUSED(dev);

	struct device *target = device_get_by_name(path + 1);
	if (!target)
		return -ENOENT;
	if (!target->ready)
		return -EINVAL;
	return target->ready();
}

CLEAR void device_install(void)
{
	devices = list_new();

	struct vfs *vfs = zalloc(sizeof(*vfs));
	vfs->type = VFS_DEVFS;
	vfs->read = devfs_read;
	vfs->ioctl = devfs_ioctl;
	vfs->perm = devfs_perm;
	vfs->ready = devfs_ready;
	struct device *dev = zalloc(sizeof(*dev));
	dev->name = "dev";
	dev->type = DEV_CHAR;
	dev->vfs = vfs;
	device_add(dev);
	vfs_mount(dev, "/dev/");

	/* vfs_list_mounts(); */
}

/**
 * EXT2
 */

// TODO: Remove malloc from buffer_read (attempt in #56cd63f199)
static void *buffer_read(u32 block, struct device *dev)
{
	void *buf = zalloc(BLOCK_SIZE);
	dev->read(buf, block * SECTOR_COUNT, SECTOR_COUNT, dev);
	return buf;
}

static struct ext2_superblock *get_superblock(struct device *dev)
{
	struct ext2_superblock *sb = buffer_read(EXT2_SUPER, dev);

	assert(sb->magic == EXT2_MAGIC);
	return sb;
}

static struct ext2_bgd *get_bgd(struct device *dev)
{
	return buffer_read(EXT2_SUPER + 1, dev);
}

static struct ext2_inode *get_inode(u32 i, struct ext2_inode *in_buf, struct device *dev)
{
	struct ext2_superblock *s = get_superblock(dev);
	assert(s);
	struct ext2_bgd *b = get_bgd(dev);
	assert(b);

	u32 block_group = (i - 1) / s->inodes_per_group;
	u32 index = (i - 1) % s->inodes_per_group;
	u32 block = (index * EXT2_INODE_SIZE) / BLOCK_SIZE;
	b += block_group;

	u32 *buf = buffer_read(b->inode_table + block, dev);
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
static u32 read_indirect(u32 indirect, u32 block_num, struct device *dev)
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
		data = buffer_read(indirect, dev);
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

static res read_inode(struct ext2_inode *in, void *buf, u32 offset, u32 count, struct device *dev)
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
			blocknum = read_indirect(indirect, i - 12, dev);
		} else {
			indirect = in->block[13];
			blocknum = read_indirect(indirect, (i - (BLOCK_COUNT + 12)) / BLOCK_COUNT,
						 dev);
			blocknum = read_indirect(blocknum, (i - (BLOCK_COUNT + 12)) % BLOCK_COUNT,
						 dev);
		}

		char *data = buffer_read(blocknum, dev);
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

static u32 find_inode(const char *name, u32 dir_inode, struct device *dev)
{
	if ((signed)dir_inode <= 0)
		return (unsigned)-1;

	struct ext2_inode i = { 0 };
	get_inode(dir_inode, &i, dev);

	char *buf = malloc(BLOCK_SIZE * i.blocks / 2);
	memset(buf, 0, BLOCK_SIZE * i.blocks / 2);

	for (u32 q = 0; q < i.blocks / 2; q++) {
		char *data = buffer_read(i.block[q], dev);
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

static struct ext2_inode *find_inode_by_path(const char *path, struct ext2_inode *in_buf,
					     struct device *dev)
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
		current_inode = find_inode(path_cp, current_inode, dev);
		path_cp[i] = '/';

		if ((signed)current_inode <= 0) {
			free(init);
			return NULL;
		}

		path_cp += i + 1;
	}

	u32 inode = find_inode(path_cp, current_inode, dev);
	free(init);
	if ((signed)inode <= 0)
		return NULL;

	return get_inode(inode, in_buf, dev);
}

res ext2_read(const char *path, void *buf, u32 offset, u32 count, struct device *dev)
{
	struct ext2_inode in = { 0 };
	if (find_inode_by_path(path, &in, dev) == &in) {
		return read_inode(&in, buf, offset, count, dev);
	} else
		return -ENOENT;
}

res ext2_stat(const char *path, struct stat *buf, struct device *dev)
{
	struct ext2_inode in = { 0 };
	if (find_inode_by_path(path, &in, dev) != &in)
		return -ENOENT;

	//u32 num_blocks = in.blocks / (BLOCK_SIZE / SECTOR_SIZE);
	//u32 sz = BLOCK_SIZE * num_blocks;

	stac();
	buf->dev_id = dev->id;
	buf->size = in.size;
	clac();

	return EOK;
}

res ext2_perm(const char *path, enum vfs_perm perm, struct device *dev)
{
	struct ext2_inode in = { 0 };
	if (find_inode_by_path(path, &in, dev) != &in)
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

res ext2_ready(const char *path, struct device *dev)
{
	UNUSED(path);
	UNUSED(dev);
	return 1;
}

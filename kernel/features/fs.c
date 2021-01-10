// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <def.h>
#include <fs.h>
#include <ide.h>
#include <mem.h>
#include <print.h>
#include <random.h>
#include <str.h>

/**
 * VFS
 */

static struct list *mount_points = NULL;

char *vfs_normalize_path(const char *path)
{
	char *fixed = strdup(path);
	int len = strlen(fixed);
	if (fixed[len - 1] == '/' && len != 1)
		fixed[len - 1] = '\0';
	return fixed;
}

u32 vfs_mounted(struct device *dev, const char *path)
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

struct mount_info *vfs_recursive_find(char *path)
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

struct mount_info *vfs_find_mount_info(const char *path)
{
	assert(path[0] == '/');
	return vfs_recursive_find(strdup(path));
}

struct device *vfs_find_dev(const char *path)
{
	assert(path[0] == '/');
	struct mount_info *m = vfs_find_mount_info(path);
	return m && m->dev ? m->dev : NULL;
}

const char *vfs_resolve_type(enum vfs_type type)
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

void vfs_list_mounts()
{
	struct node *iterator = mount_points->head;
	while (iterator) {
		struct mount_info *m = iterator->data;
		printf("%s on %s type %s\n", m->dev->name, m->path,
		       vfs_resolve_type(m->dev->vfs->type));
		iterator = iterator->next;
	}
}

u32 vfs_mount(struct device *dev, const char *path)
{
	// TODO: Check if already mounted
	if (!dev || !dev->id || vfs_mounted(dev, path))
		return 0;

	char *fixed = vfs_normalize_path(path);

	struct mount_info *m = malloc(sizeof(*m));
	m->path = fixed;
	m->dev = dev;
	list_add(mount_points, m);

	return 1;
}

u32 vfs_read(const char *path, void *buf, u32 offset, u32 count)
{
	if (count == 0 || offset > count)
		return 0;

	struct mount_info *m = vfs_find_mount_info(path);
	assert(m && m->dev && m->dev->vfs && m->dev->vfs->read);

	u32 len = strlen(m->path);
	if (len > 1)
		path += len;

	struct device *dev = m->dev;
	return dev->vfs->read(path, buf, offset, count, dev);
}

u32 vfs_write(const char *path, void *buf, u32 offset, u32 count)
{
	struct device *dev = vfs_find_dev(path);
	assert(dev && dev->vfs && dev->vfs->write);
	return dev->vfs->write(path, buf, offset, count, dev);
}

u32 vfs_stat(const char *path, struct stat *buf)
{
	struct device *dev = vfs_find_dev(path);
	assert(dev && dev->vfs && dev->vfs->stat);
	return dev->vfs->stat(path, buf, dev);
}

void vfs_install(void)
{
	mount_points = list_new();
}

/**
 * Device
 */

static struct list *devices = NULL;

void device_add(struct device *dev)
{
	dev->id = rand() + 1;
	list_add(devices, dev);
}

struct device *device_get(u32 id)
{
	struct node *iterator = devices->head;
	while (iterator) {
		if (((struct device *)iterator->data)->id == id)
			return iterator->data;
		iterator = iterator->next;
	}
	return NULL;
}

u32 devfs_read(const char *path, void *buf, u32 offset, u32 count, struct device *dev)
{
	assert(dev && dev->read);
	printf("%s - off: %d, cnt: %d, buf: %x, dev %x\n", path, offset, count, buf, dev);
	return dev->read(buf, offset, count, dev);
}

void device_install(void)
{
	devices = list_new();

	struct vfs *vfs = malloc(sizeof(*vfs));
	vfs->type = VFS_DEVFS;
	vfs->read = devfs_read;
	struct device *dev = malloc(sizeof(*dev));
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

void *buffer_read(u32 block, struct device *dev)
{
	void *buf = malloc(BLOCK_SIZE);
	dev->read(buf, block * SECTOR_COUNT, SECTOR_COUNT, dev);
	return buf;
}

struct ext2_superblock *get_superblock(struct device *dev)
{
	struct ext2_superblock *sb = buffer_read(EXT2_SUPER, dev);

	if (sb->magic != EXT2_MAGIC)
		return NULL;
	return sb;
}

struct ext2_bgd *get_bgd(struct device *dev)
{
	return buffer_read(EXT2_SUPER + 1, dev);
}

struct ext2_inode *get_inode(u32 i, struct device *dev)
{
	struct ext2_superblock *s = get_superblock(dev);
	assert(s);
	struct ext2_bgd *b = get_bgd(dev);
	assert(b);

	u32 block_group = (i - 1) / s->inodes_per_group;
	u32 index = (i - 1) % s->inodes_per_group;
	u32 block = (index * EXT2_INODE_SIZE) / BLOCK_SIZE;
	b += block_group;

	u32 *data = buffer_read(b->inode_table + block, dev);
	struct ext2_inode *in =
		(struct ext2_inode *)((u32)data +
				      (index % (BLOCK_SIZE / EXT2_INODE_SIZE)) * EXT2_INODE_SIZE);
	return in;
}

u32 read_indirect(u32 indirect, u32 block_num, struct device *dev)
{
	char *data = buffer_read(indirect, dev);
	return *(u32 *)((u32)data + block_num * sizeof(u32));
}

u32 read_inode(struct ext2_inode *in, void *buf, u32 offset, u32 count, struct device *dev)
{
	// TODO: Support read offset
	(void)offset;

	if (!in || !buf)
		return 0;

	u32 num_blocks = in->blocks / (BLOCK_SIZE / SECTOR_SIZE);

	if (!num_blocks)
		return 0;

	// TODO: memcpy block chunks until count is copied
	while (BLOCK_SIZE * num_blocks > count)
		num_blocks--;

	u32 indirect = 0;
	u32 blocknum = 0;
	char *data = 0;
	// TODO: Support triply indirect pointers
	// TODO: This can be heavily optimized by saving the indirect block lists
	for (u32 i = 0; i < num_blocks; i++) {
		if (i < 12) {
			blocknum = in->block[i];
			data = buffer_read(blocknum, dev);
			memcpy((u32 *)((u32)buf + i * BLOCK_SIZE), data, BLOCK_SIZE);
		} else if (i < BLOCK_COUNT + 12) {
			indirect = in->block[12];
			blocknum = read_indirect(indirect, i - 12, dev);
			data = buffer_read(blocknum, dev);
			memcpy((u32 *)((u32)buf + i * BLOCK_SIZE), data, BLOCK_SIZE);
		} else {
			indirect = in->block[13];
			blocknum = read_indirect(indirect, (i - (BLOCK_COUNT + 12)) / BLOCK_COUNT,
						 dev);
			blocknum = read_indirect(blocknum, (i - (BLOCK_COUNT + 12)) % BLOCK_COUNT,
						 dev);
			data = buffer_read(blocknum, dev);
			memcpy((u32 *)((u32)buf + i * BLOCK_SIZE), data, BLOCK_SIZE);
		}
		/* printf("Loaded %d of %d\n", i + 1, num_blocks); */
	}

	return count;
}

u32 find_inode(const char *name, u32 dir_inode, struct device *dev)
{
	if (!dir_inode)
		return (unsigned)-1;

	struct ext2_inode *i = get_inode(dir_inode, dev);

	char *buf = malloc(BLOCK_SIZE * i->blocks / 2);
	memset(buf, 0, BLOCK_SIZE * i->blocks / 2);

	for (u32 q = 0; q < i->blocks / 2; q++) {
		char *data = buffer_read(i->block[q], dev);
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

	} while (sum < (1024 * i->blocks / 2));
	free(buf);
	return (unsigned)-1;
}

struct ext2_inode *find_inode_by_path(const char *path, struct device *dev)
{
	if (path[0] != '/')
		return 0;

	char *path_cp = strdup(path);
	char *init = path_cp; // For freeing

	path_cp++;
	u32 current_inode = EXT2_ROOT;

	int i = 0;
	while (1) {
		for (i = 0; path_cp[i] != '/' && path_cp[i] != '\0'; i++)
			;

		if (path_cp[i] == '\0')
			break;

		path_cp[i] = '\0';
		current_inode = find_inode(path_cp, current_inode, dev);
		path_cp[i] = '/';

		if (current_inode == 0) {
			free(init);
			return 0;
		}

		path_cp += i + 1;
	}

	u32 inode = find_inode(path_cp, current_inode, dev);
	free(init);
	if ((signed)inode <= 0)
		return 0;

	return get_inode(inode, dev);
}

u32 ext2_read(const char *path, void *buf, u32 offset, u32 count, struct device *dev)
{
	struct ext2_inode *in = find_inode_by_path(path, dev);
	if (in)
		return read_inode(in, buf, offset, count, dev);
	else
		return 0;
}

u32 ext2_stat(const char *path, struct stat *buf, struct device *dev)
{
	if (!buf)
		return 1;

	struct ext2_inode *in = find_inode_by_path(path, dev);
	if (!in)
		return 1;

	u32 num_blocks = in->blocks / (BLOCK_SIZE / SECTOR_SIZE);
	u32 sz = BLOCK_SIZE * num_blocks;

	buf->dev_id = dev->id;
	buf->size = sz; // Actually in->size but ext2..

	return 0;
}

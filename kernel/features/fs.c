// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <def.h>
#include <fs.h>
#include <ide.h>
#include <mem.h>
#include <print.h>
#include <random.h>
#include <str.h>

#include <cpu.h> // TODO: Remove later

/**
 * VFS
 */

static struct list *mount_points = NULL;

struct device *vfs_mounted(const char *path)
{
	struct node *iterator = mount_points->head;
	while (iterator) {
		struct mount_info *m = iterator->data;
		if (!strncmp(m->path, path, strlen(m->path)))
			return m->dev;
		iterator = iterator->next;
	}
	return NULL;
}

u32 vfs_mount(struct device *dev, const char *path)
{
	if (!dev || !dev->id || vfs_mounted(path))
		return 0;

	struct mount_info *m = malloc(sizeof(*m));
	m->path = strdup(path);
	m->dev = dev;
	list_add(mount_points, m);

	return 1;
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

void *vfs_read(char *path)
{
	(void)path;
	printf("NOT IMPLEMENTED!\n");
	loop();
	return NULL;
}

u32 vfs_stat(char *path)
{
	(void)path;
	printf("NOT IMPLEMENTED!\n");
	loop();
	return 0;
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

void device_install(void)
{
	devices = list_new();

	struct vfs *vfs = malloc(sizeof(*vfs));
	vfs->type = VFS_DEVFS;
	struct device *dev = malloc(sizeof(*dev));
	dev->name = "dev";
	dev->vfs = vfs;
	device_add(dev);
	vfs_mount(dev, "/dev/");
	printf("%s\n", vfs_mounted("/dev/test")->name);
	vfs_list_mounts();
}

/**
 * EXT2
 */

void *buffer_read(u32 block)
{
	return ide_read(malloc(BLOCK_SIZE), block);
}

struct ext2_superblock *get_superblock(void)
{
	struct ext2_superblock *sb = buffer_read(EXT2_SUPER);
	if (sb->magic != EXT2_MAGIC)
		return NULL;
	return sb;
}

struct ext2_bgd *get_bgd(void)
{
	return buffer_read(EXT2_SUPER + 1);
}

struct ext2_inode *get_inode(u32 i)
{
	struct ext2_superblock *s = get_superblock();
	assert(s);
	struct ext2_bgd *b = get_bgd();
	assert(b);

	u32 block_group = (i - 1) / s->inodes_per_group;
	u32 index = (i - 1) % s->inodes_per_group;
	u32 block = (index * EXT2_INODE_SIZE) / BLOCK_SIZE;
	b += block_group;

	u32 *data = buffer_read(b->inode_table + block);
	struct ext2_inode *in =
		(struct ext2_inode *)((u32)data +
				      (index % (BLOCK_SIZE / EXT2_INODE_SIZE)) * EXT2_INODE_SIZE);
	return in;
}

u32 read_indirect(u32 indirect, u32 block_num)
{
	char *data = buffer_read(indirect);
	return *(u32 *)((u32)data + block_num * sizeof(u32));
}

void *read_inode(struct ext2_inode *in)
{
	assert(in);
	if (!in)
		return NULL;

	u32 num_blocks = in->blocks / (BLOCK_SIZE / SECTOR_SIZE);

	assert(num_blocks != 0);
	if (!num_blocks)
		return NULL;

	/* u32 sz = BLOCK_SIZE * num_blocks; */
	u32 sz = in->size;
	void *buf = malloc(sz);
	printf("Loading %dKiB\n", sz >> 10);
	assert(buf != NULL);

	u32 indirect = 0;
	u32 blocknum = 0;
	char *data = 0;
	// TODO: Support triply indirect pointers
	// TODO: This can be heavily optimized by saving the indirect block lists
	for (u32 i = 0; i < num_blocks; i++) {
		if (i < 12) {
			blocknum = in->block[i];
			data = buffer_read(blocknum);
			memcpy((u32 *)((u32)buf + i * BLOCK_SIZE), data, BLOCK_SIZE);
		} else if (i < BLOCK_COUNT + 12) {
			indirect = in->block[12];
			blocknum = read_indirect(indirect, i - 12);
			data = buffer_read(blocknum);
			memcpy((u32 *)((u32)buf + i * BLOCK_SIZE), data, BLOCK_SIZE);
		} else {
			indirect = in->block[13];
			blocknum = read_indirect(indirect, (i - (BLOCK_COUNT + 12)) / BLOCK_COUNT);
			blocknum = read_indirect(blocknum, (i - (BLOCK_COUNT + 12)) % BLOCK_COUNT);
			data = buffer_read(blocknum);
			memcpy((u32 *)((u32)buf + i * BLOCK_SIZE), data, BLOCK_SIZE);
		}
		/* printf("Loaded %d of %d\n", i + 1, num_blocks); */
	}

	return buf;
}

u32 find_inode(const char *name, u32 dir_inode)
{
	if (!dir_inode)
		return (unsigned)-1;

	struct ext2_inode *i = get_inode(dir_inode);

	char *buf = malloc(BLOCK_SIZE * i->blocks / 2);
	memset(buf, 0, BLOCK_SIZE * i->blocks / 2);

	for (u32 q = 0; q < i->blocks / 2; q++) {
		char *data = buffer_read(i->block[q]);
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

struct ext2_inode *find_inode_by_path(char *path)
{
	if (path[0] != '/')
		return 0;

	path++;
	u32 current_inode = EXT2_ROOT;

	int i = 0;
	while (1) {
		for (i = 0; path[i] != '/' && path[i] != '\0'; i++)
			;

		if (path[i] == '\0')
			break;

		path[i] = '\0';
		current_inode = find_inode(path, current_inode);
		path[i] = '/';

		if (current_inode == 0)
			return 0;

		path += i + 1;
	}

	u32 inode = find_inode(path, current_inode);
	if ((signed)inode <= 0)
		return 0;

	return get_inode(inode);
}

void *ext2_read(char *path)
{
	struct ext2_inode *in = find_inode_by_path(path);
	if (in)
		return read_inode(in);
	else
		return NULL;
}

u32 ext2_stat(char *path)
{
	struct ext2_inode *in = find_inode_by_path(path);
	if (!in)
		return 0;

	return in->size;
}

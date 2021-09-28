// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <list.h>
#include <mem.h>
#include <print.h>
#include <str.h>

#include <core/protection.h>
#include <fs/ext2.h>
#include <fs/virtual.h>

// TODO: Move!
#define BLOCK_SIZE 1024
#define BLOCK_COUNT 256 // BLOCK_SIZE / sizeof(u32)
#define SECTOR_SIZE 512
#define SECTOR_COUNT (BLOCK_SIZE / SECTOR_SIZE) // 2

static void ext2_buffer_read(u32 block, void *buf, struct vfs_dev *dev)
{
	dev->read(buf, block * SECTOR_COUNT, SECTOR_COUNT, dev);
}

static u8 ext2_verify(struct vfs_dev *dev)
{
	u8 data[BLOCK_SIZE] = { 0 };
	ext2_buffer_read(EXT2_SUPER, data, dev);
	struct ext2_superblock *sb = (struct ext2_superblock *)data;
	return sb->magic == EXT2_MAGIC;
}

static struct ext2_superblock *ext2_superblock(struct vfs_dev *dev)
{
	struct ext2_superblock *sb;

	if (dev->vfs->data) {
		sb = dev->vfs->data;
	} else {
		sb = malloc(BLOCK_SIZE); // TODO: Destroy malloced superblock?
		ext2_buffer_read(EXT2_SUPER, sb, dev);
		dev->vfs->data = sb;
	}

	assert(sb->magic == EXT2_MAGIC);
	return sb;
}

static u32 ext2_inode_size(struct vfs_dev *dev)
{
	struct ext2_superblock *sb = ext2_superblock(dev);
	if (sb->major_version == 1)
		return sb->inode_size;
	return 128; // Or 256?
}

static struct ext2_inode *ext2_inode(u32 i, struct ext2_inode *in_buf, struct vfs_dev *dev)
{
	struct ext2_superblock *sb = ext2_superblock(dev);
	u32 inode_size = ext2_inode_size(dev);

	u8 data[BLOCK_SIZE] = { 0 };
	ext2_buffer_read(EXT2_SUPER + 1, data, dev);
	struct ext2_bgd *bgd = (struct ext2_bgd *)data;

	u32 block_group = (i - 1) / sb->inodes_per_group;
	u32 index = (i - 1) % sb->inodes_per_group;
	u32 block = (index * inode_size) / BLOCK_SIZE;
	bgd += block_group;

	u8 buf[BLOCK_SIZE] = { 0 };
	ext2_buffer_read(bgd->inode_table + block, buf, dev);
	struct ext2_inode *in =
		(struct ext2_inode *)((u32)buf + (index % (BLOCK_SIZE / inode_size)) * inode_size);

	memcpy(in_buf, in, inode_size);

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

		EXPOSE(memcpy, (u8 *)buf + copied, data + block_offset, byte_count);

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
	ext2_inode(dir_inode, &i, dev);

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
	} while (sum < (BLOCK_SIZE * i.blocks / 2));

	free(buf);
	return (unsigned)-1;
}

static struct ext2_inode *ext2_find_inode_by_path(const char *path, struct ext2_inode *in_buf,
						  struct vfs_dev *dev)
{
	char *path_cp = EXPOSE(strdup, path);
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

	return ext2_inode(inode, in_buf, dev);
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

TEMPORARY u8 ext2_load(struct vfs_dev *dev)
{
	if (!ext2_verify(dev))
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

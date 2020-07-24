// MIT License, Copyright (c) 2020 Marvin Borner
// EXT2 based filesystem

#include <def.h>
#include <fs.h>
#include <ide.h>
#include <mem.h>
#include <print.h>
#include <str.h>

void *buffer_read(int block)
{
	return ide_read(malloc(BLOCK_SIZE), block);
}

struct superblock *get_superblock()
{
	struct superblock *sb = buffer_read(EXT2_SUPER);
	if (sb->magic != EXT2_MAGIC)
		return NULL;
	return sb;
}

struct bgd *get_bgd()
{
	return buffer_read(EXT2_SUPER + 1);
}

struct inode *get_inode(int i)
{
	struct superblock *s = get_superblock();
	struct bgd *b = get_bgd();

	int block_group = (i - 1) / s->inodes_per_group;
	int index = (i - 1) % s->inodes_per_group;
	int block = (index * INODE_SIZE) / BLOCK_SIZE;
	b += block_group;

	u32 *data = buffer_read(b->inode_table + block);
	struct inode *in =
		(struct inode *)((u32)data + (index % (BLOCK_SIZE / INODE_SIZE)) * INODE_SIZE);
	return in;
}

u32 read_indirect(u32 indirect, u32 block_num)
{
	char *data = buffer_read(indirect);
	return *(u32 *)((u32)data + block_num * 4);
}

void *read_inode(struct inode *in)
{
	//assert(in);
	if (!in)
		return NULL;

	int num_blocks = in->blocks / (BLOCK_SIZE / SECTOR_SIZE);

	//assert(num_blocks != 0);
	if (!num_blocks)
		return NULL;

	u32 sz = BLOCK_SIZE * num_blocks;
	void *buf = malloc(sz);
	printf("Loading %dKiB\n", sz >> 10);
	//assert(buf != NULL);

	int indirect;

	// Singly indirect pointer
	// TODO: Support doubly and triply pointers
	if (num_blocks > 12)
		indirect = in->block[12];

	int blocknum = 0;
	char *data;
	for (int i = 0; i < num_blocks; i++) {
		if (i < 12) {
			blocknum = in->block[i];
			data = buffer_read(blocknum);
			memcpy((u8 *)((u32)buf + i * BLOCK_SIZE), data, BLOCK_SIZE);
		} else {
			blocknum = read_indirect(indirect, i - 12);
			data = buffer_read(blocknum);
			memcpy((u8 *)((u32)buf + (i - 1) * BLOCK_SIZE), data, BLOCK_SIZE);
		}
	}

	return buf;
}

void *read_file(const char *name, int dir_inode)
{
	return read_inode(get_inode(find_inode(name, dir_inode)));
}

int find_inode(const char *name, int dir_inode)
{
	if (!dir_inode)
		return -1;

	struct inode *i = get_inode(dir_inode);

	char *buf = malloc(BLOCK_SIZE * i->blocks / 2);
	memset(buf, 0, BLOCK_SIZE * i->blocks / 2);

	for (u32 q = 0; q < i->blocks / 2; q++) {
		char *data = buffer_read(i->block[q]);
		memcpy((u32 *)((u32)buf + q * BLOCK_SIZE), data, BLOCK_SIZE);
	}

	struct dirent *d = (struct dirent *)buf;

	u32 sum = 0;
	do {
		// Calculate the 4byte aligned size of each entry
		sum += d->total_len;
		if (strncmp((void *)d->name, name, d->name_len) == 0) {
			free(buf);
			return d->inode_num;
		}
		d = (struct dirent *)((u32)d + d->total_len);

	} while (sum < (1024 * i->blocks / 2));
	free(buf);
	return -1;
}

void ls_root()
{
	struct inode *i = get_inode(2);

	char *buf = malloc(BLOCK_SIZE * i->blocks / 2);

	for (u32 q = 0; q < i->blocks / 2; q++) {
		char *data = buffer_read(i->block[q]);
		memcpy((u32 *)((u32)buf + q * BLOCK_SIZE), data, BLOCK_SIZE);
	}

	struct dirent *d = (struct dirent *)buf;

	int sum = 0;
	int calc = 0;
	printf("\nRoot directory:\n");
	do {
		calc = (sizeof(struct dirent) + d->name_len + 4) & ~0x3;
		sum += d->total_len;
		printf("/%s\n", d->name);
		if (d->total_len != calc && sum == 1024)
			d->total_len = calc;

		d = (struct dirent *)((u32)d + d->total_len);

	} while (sum < 1024);
	printf("\n");
}
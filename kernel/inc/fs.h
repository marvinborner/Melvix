// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef FS_H
#define FS_H

#include <def.h>

/**
 * Device
 */

struct device {
	u32 id;
	const char *name;
	int type; // TODO: Block, char device
	struct vfs *vfs;
	u8 (*read)(u8 *buf, u32 offset, u32 count, struct device *dev);
	u8 (*write)(u8 *buf, u32 offset, u32 count, struct device *dev);
};

/**
 * VFS
 */

struct vfs {
	const char *name;
	u8 (*read)(char *, char *, struct device *, void *);
	u8 (*mount)(struct device *, void *);
};

struct mount_info {
	const char *path;
	struct device *dev;
};

/**
 * EXT2
 */

#define EXT2_BOOT 0
#define EXT2_SUPER 1
#define EXT2_ROOT 2
#define EXT2_MAGIC 0x0000EF53

struct superblock {
	u32 total_inodes;
	u32 total_blocks;
	u32 su_res_blocks; // Superuser reserved
	u32 free_blocks;
	u32 free_inodes;
	u32 superblock_block_num;
	u32 log2_block_size;
	u32 log2_frag_size;
	u32 blocks_per_group;
	u32 frags_per_group;
	u32 inodes_per_group;
	u32 last_mount_time;
	u32 last_write_time;
	u16 mounts_since_fsck;
	u16 max_mounts_since_fsck;
	u16 magic;
	u16 state; // 1 clean; 2 errors
	u16 error_action;
	u16 minor_version;
	u32 last_fsck_time;
	u32 max_time_since_fsck;
	u32 creator_os_id;
	u32 major_version;
	u16 res_block_uid;
	u16 res_block_gid;
};

struct bgd {
	u32 block_bitmap;
	u32 inode_bitmap;
	u32 inode_table;
	u16 free_blocks;
	u16 free_inodes;
	u16 used_dirs;
	u16 pad;
	u8 bg_reserved[12];
};

struct inode {
	u16 mode;
	u16 uid;
	u32 size;

	u32 last_access_time;
	u32 creation_time;
	u32 last_modification_time;
	u32 deletion_time;

	u16 gid;
	u16 link_count;
	u32 blocks;
	u32 flags;
	u32 os_specific_val1;
	u32 block[15];
	u32 generation;

	u32 reserved1;
	u32 reserved2;

	u32 fragment_addr;
	u8 os_specific_val2[12];
};

#define INODE_SIZE (sizeof(struct inode))

struct dirent {
	u32 inode_num;
	u16 total_len;
	u8 name_len;
	u8 type_indicator;
	u8 name[];
};

struct file {
	struct inode inode;
	u32 pos;
	u8 block_index;
	u8 *buf;
	u32 curr_block_pos;
};

void *file_read(char *path);
u32 file_stat(char *path);

#endif

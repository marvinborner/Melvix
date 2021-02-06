// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef FS_H
#define FS_H

#include <def.h>
#include <sys.h>

/**
 * Device
 */

enum dev_type { DEV_BLOCK, DEV_CHAR };

struct device {
	u32 id;
	const char *name;
	enum dev_type type;
	struct vfs *vfs;
	void *data;
	s32 (*read)(void *buf, u32 offset, u32 count, struct device *dev);
	s32 (*write)(void *buf, u32 offset, u32 count, struct device *dev);
	u8 (*ready)();
};

void device_install(void);

void device_add(struct device *dev);

/**
 * VFS
 */

enum vfs_type { VFS_DEVFS, VFS_TMPFS, VFS_PROCFS, VFS_EXT2 };

struct vfs {
	enum vfs_type type;
	int flags;
	void *data;
	s32 (*read)(const char *path, void *buf, u32 offset, u32 count, struct device *dev);
	s32 (*write)(const char *path, void *buf, u32 offset, u32 count, struct device *dev);
	s32 (*stat)(const char *path, struct stat *buf, struct device *dev);
	u8 (*ready)(const char *path, struct device *dev);
};

struct mount_info {
	const char *path;
	struct device *dev;
};

void vfs_install(void);

u8 vfs_mounted(struct device *dev, const char *path);
s32 vfs_mount(struct device *dev, const char *path);

struct device *vfs_find_dev(const char *path);

s32 vfs_read(const char *path, void *buf, u32 offset, u32 count);
s32 vfs_write(const char *path, void *buf, u32 offset, u32 count);
s32 vfs_stat(const char *path, struct stat *buf);
u8 vfs_ready(const char *path);

struct device *device_get_by_name(const char *name);

/**
 * EXT2
 */

#define EXT2_BOOT 0
#define EXT2_SUPER 1
#define EXT2_ROOT 2
#define EXT2_MAGIC 0x0000EF53

struct ext2_superblock {
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

struct ext2_bgd {
	u32 block_bitmap;
	u32 inode_bitmap;
	u32 inode_table;
	u16 free_blocks;
	u16 free_inodes;
	u16 used_dirs;
	u16 pad;
	u8 bg_reserved[12];
};

struct ext2_inode {
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

#define EXT2_INODE_SIZE (sizeof(struct ext2_inode))

struct ext2_dirent {
	u32 inode_num;
	u16 total_len;
	u8 name_len;
	u8 type_indicator;
	u8 name[];
};

struct ext2_file {
	struct ext2_inode inode;
	u32 pos;
	u8 block_index;
	u8 *buf;
	u32 curr_block_pos;
};

s32 ext2_read(const char *path, void *buf, u32 offset, u32 count, struct device *dev);
s32 ext2_stat(const char *path, struct stat *buf, struct device *dev);
u8 ext2_ready(const char *path, struct device *dev);

#endif

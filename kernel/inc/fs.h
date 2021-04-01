// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef FS_H
#define FS_H

#include <def.h>
#include <errno.h>
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
	res (*read)(void *buf, u32 offset, u32 count, struct device *dev) NONNULL;
	res (*write)(void *buf, u32 offset, u32 count, struct device *dev) NONNULL;
	res (*ioctl)(u32 request, void *arg1, void *arg2, void *arg3, struct device *dev)
		ATTR((nonnull(5)));
	res (*ready)(void);
};

void device_install(void);

void device_add(struct device *dev) NONNULL;

/**
 * VFS
 */

enum vfs_type { VFS_DEVFS, VFS_TMPFS, VFS_PROCFS, VFS_EXT2 };
enum vfs_perm { VFS_EXEC, VFS_WRITE, VFS_READ };

struct vfs {
	enum vfs_type type;
	int flags;
	void *data;
	res (*read)(const char *path, void *buf, u32 offset, u32 count, struct device *dev) NONNULL;
	res (*write)(const char *path, void *buf, u32 offset, u32 count,
		     struct device *dev) NONNULL;
	res (*ioctl)(const char *path, u32 request, void *arg1, void *arg2, void *arg3,
		     struct device *dev) ATTR((nonnull(1, 6)));
	res (*stat)(const char *path, struct stat *buf, struct device *dev) NONNULL;
	res (*block)(const char *path, u32 func_ptr, struct device *dev) NONNULL;
	res (*ready)(const char *path, struct device *dev) NONNULL;
	res (*perm)(const char *path, enum vfs_perm perm, struct device *dev) NONNULL;
};

struct mount_info {
	const char *path;
	struct device *dev;
};

void vfs_install(void);

u8 vfs_mounted(struct device *dev, const char *path) NONNULL;
res vfs_mount(struct device *dev, const char *path) NONNULL;

struct device *vfs_find_dev(const char *path) NONNULL;

res vfs_read(const char *path, void *buf, u32 offset, u32 count) NONNULL;
res vfs_write(const char *path, void *buf, u32 offset, u32 count) NONNULL;
res vfs_ioctl(const char *path, u32 request, void *arg1, void *arg2, void *arg3) ATTR((nonnull(1)));
res vfs_stat(const char *path, struct stat *buf) NONNULL;
res vfs_block(const char *path, u32 func_ptr) NONNULL;
res vfs_poll(const char **files) NONNULL;
res vfs_ready(const char *path) NONNULL;

struct device *device_get_by_name(const char *name) NONNULL;
struct device *device_get_by_id(u32 id) NONNULL;

/**
 * EXT2
 */

#define EXT2_BOOT 0
#define EXT2_SUPER 1
#define EXT2_ROOT 2
#define EXT2_MAGIC 0x0000EF53

// TODO: Support other and group permissions?
#define EXT2_PERM_OEXEC 0x001
#define EXT2_PERM_OWRITE 0x002
#define EXT2_PERM_OREAD 0x004
#define EXT2_PERM_GEXEC 0x008
#define EXT2_PERM_GWRITE 0x010
#define EXT2_PERM_GREAD 0x020
#define EXT2_PERM_UEXEC 0x040
#define EXT2_PERM_UWRITE 0x080
#define EXT2_PERM_UREAD 0x100

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

res ext2_read(const char *path, void *buf, u32 offset, u32 count, struct device *dev) NONNULL;
res ext2_stat(const char *path, struct stat *buf, struct device *dev) NONNULL;
res ext2_perm(const char *path, enum vfs_perm perm, struct device *dev) NONNULL;
res ext2_ready(const char *path, struct device *dev) NONNULL;

#endif

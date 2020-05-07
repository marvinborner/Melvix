#ifndef MELVIX_EXT2_H
#define MELVIX_EXT2_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ROOT_INODE 2

#define EXT2_SIGNATURE 0xEF53
#define INODE_SIZE 128

#define SUPERBLOCK_OFFSET 1024
#define SUPERBLOCK_LENGTH 1024

#define SUPERBLOCK_LBA (SUPERBLOCK_OFFSET / SECTOR_SIZE)
#define SUPERBLOCK_SECTORS (SUPERBLOCK_LENGTH / SECTOR_SIZE)

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
	u16 signature;
	u16 state; // 1 clean; 2 errors
	u16 error_action;
	u16 minor_version;
	u32 last_fsck_time;
	u32 max_time_since_fsck;
	u32 creator_os_id;
	u32 major_version;
	u16 res_block_uid;
	u16 res_block_gid;
} __attribute__((packed));

// Block group descriptor
struct bgd {
	u32 block_bitmap_addr;
	u32 inode_bitmap_addr;
	u32 inode_table_addr;
	u16 free_blocks;
	u16 free_inodes;
	u16 used_dirs;
	u16 pad;
	u8 bg_reserved[12];
} __attribute__((packed));

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
	u32 sectors_used;
	u32 flags;
	u32 os_specific_val1;
	u32 dbp[12];
	u32 ibp;
	u32 dibp;
	u32 tibp;
	u32 gen_number;

	u32 reserved1;
	u32 reserved2;

	u32 fragment_addr;
	u8 os_specific_val2[12];
} __attribute__((packed));

#define S_IFIFO 0x1000
#define S_IFCHR 0x2000
#define S_IFDIR 0x4000
#define S_IFBLK 0x6000
#define S_IFREG 0x8000
#define S_IFLNK 0xA000
#define S_IFSOCK 0xC000

#define S_ISDIR(m) ((m & 0170000) == 0040000)
#define S_ISCHR(m) ((m & 0170000) == 0020000)
#define S_ISBLK(m) ((m & 0170000) == 0060000)
#define S_ISREG(m) ((m & 0170000) == 0100000)
#define S_ISFIFO(m) ((m & 0170000) == 0010000)
#define S_ISLNK(m) ((m & 0170000) == 0120000)
#define S_ISSOCK(m) ((m & 0170000) == 0140000)

#define S_ISUID 04000
#define S_ISGID 02000
#define S_ISTICK 01000
#define S_IRUSR 00400
#define S_IWUSR 00200
#define S_IXUSR 00100
#define S_IRGRP 00040
#define S_IWGRP 00020
#define S_IXGRP 00010
#define S_IROTH 00004
#define S_IWOTH 00002
#define S_IXOTH 00001

#define SECURE_DELETE 0x00001
#define UNDELETE 0x00002
#define COMPRESSED 0x00004
#define SYNCRONOUS 0x00008
#define IMMUTABLE 0x00010
#define APPEND_ONLY 0x00020
#define DUMP_IGNORE 0x00040
#define NO_UPDATE_ACCESS 0x00080

struct fs_node *ext2_root;

struct ext2_dirent {
	u32 inode_num;
	u16 total_len;
	u8 name_len;
	u8 type_indicator;
	u8 *name;
} __attribute__((packed));

struct ext2_file {
	struct ext2_inode inode;
	u32 pos;
	u8 block_index;
	u8 *buf;
	u32 curr_block_pos;
};

void ext2_init_fs();
void ext2_open_inode(u32 inode_num, struct ext2_file *file);
u32 ext2_read(struct ext2_file *file, u8 *buf, u32 count);
bool ext2_next_dirent(struct ext2_file *file, struct ext2_dirent *dir);
u32 ext2_find_in_dir(u32 dir_inode, const char *name);
u32 ext2_look_up_path(char *path);

u8 *read_file(char *path);
void ext2_node_init(struct fs_node *node);
void ext2_mount(struct fs_node *mountpoint);

#endif
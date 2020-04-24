#ifndef MELVIX_EXT2_H
#define MELVIX_EXT2_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define ROOT_INODE 2

#define EXT2_SIGNATURE 0xEF53
#define INODE_SIZE 128

#define SUPERBLOCK_OFFSET 1024
#define SUPERBLOCK_LENGTH 1024

#define SUPERBLOCK_LBA (SUPERBLOCK_OFFSET / SECTOR_SIZE)
#define SUPERBLOCK_SECTORS (SUPERBLOCK_LENGTH / SECTOR_SIZE)

struct ext2_superblock {
	uint32_t total_inodes;
	uint32_t total_blocks;
	uint32_t su_res_blocks; // Superuser reserved
	uint32_t free_blocks;
	uint32_t free_inodes;
	uint32_t superblock_block_num;
	uint32_t log2_block_size;
	uint32_t log2_frag_size;
	uint32_t blocks_per_group;
	uint32_t frags_per_group;
	uint32_t inodes_per_group;
	uint32_t last_mount_time;
	uint32_t last_write_time;
	uint16_t mounts_since_fsck;
	uint16_t max_mounts_since_fsck;
	uint16_t signature;
	uint16_t state;
	uint16_t error_action;
	uint16_t minor_version;
	uint32_t last_fsck_time;
	uint32_t max_time_since_fsck;
	uint32_t creator_OS_id;
	uint32_t major_version;
	uint16_t res_block_uid;
	uint16_t res_block_gid;
} __attribute__((packed));

// Block group descriptor
struct bgd {
	uint32_t block_bitmap_addr;
	uint32_t inode_bitmap_addr;
	uint32_t inode_table_addr;
	uint16_t free_blocks;
	uint16_t free_inodes;
	uint16_t used_dirs;
	uint16_t pad;
	uint8_t bg_reserved[12];
} __attribute__((packed));

struct ext2_inode {
	uint16_t type_and_permissions;
	uint16_t uid;
	uint32_t size;

	uint32_t last_access_time;
	uint32_t creation_time;
	uint32_t last_modification_time;
	uint32_t deletion_time;

	uint16_t gid;
	uint16_t link_count;
	uint32_t sectors_used;
	uint32_t flags;
	uint32_t os_specific_val1;
	uint32_t dbp[12];
	uint32_t ibp;
	uint32_t dibp;
	uint32_t tibp;
	uint32_t gen_number;

	uint32_t reserved1;
	uint32_t reserved2;

	uint32_t fragment_addr;
	uint8_t os_specific_val2[12];
} __attribute__((packed));

#define S_IFIFO 0x1000
#define S_IFCHR 0x2000
#define S_IFDIR 0x4000
#define S_IFBLK 0x6000
#define S_IFREG 0x8000
#define S_IFLNK 0xA000
#define S_IFSOCK 0xC000

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

struct ext2_dirent {
	uint32_t inode_num;
	uint16_t total_len;
	uint8_t name_len;
	uint8_t type_indicator;
	uint8_t *name;
} __attribute__((packed));

struct ext2_file {
	struct ext2_inode inode;
	size_t pos;
	uint8_t block_index;
	uint8_t *buf;
	size_t curr_block_pos;
};

void ext2_init_fs();
void ext2_open_inode(uint32_t inode_num, struct ext2_file *file);
size_t ext2_read(struct ext2_file *file, uint8_t *buf, size_t count);
bool ext2_next_dirent(struct ext2_file *file, struct ext2_dirent *dir);
uint32_t ext2_find_in_dir(uint32_t dir_inode, const char *name);
uint32_t ext2_look_up_path(char *path);

#endif

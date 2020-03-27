#ifndef MELVIX_MARFS_H
#define MELVIX_MARFS_H

#include <stdint.h>

struct marfs_superblock {
	uint64_t signature;
	uint32_t n_inodes;
	uint32_t n_chunks;
	uint32_t n_first_unallocated_inode;
	uint32_t s_first_inode;
	uint32_t s_first_chunk;
} __attribute__((packed));

struct marfs_inode {
	uint64_t size;
	uint32_t creation_time;
	uint32_t last_mod_time;
	uint32_t last_access_time;
	uint32_t n_blocks;
	uint32_t DBPs[10];
	uint32_t ext_1;
	uint32_t ext_2;
	uint32_t ext_3;
	uint32_t ext_4;
	uint32_t uid;
	uint8_t is_app;
	uint8_t is_dir;
	uint8_t is_used;
} __attribute__((packed));

struct ata_interface *interface;
struct marfs_superblock sb_cache;
uint32_t max_lba;

// marfs_sectorlevel.c
uint8_t marfs_init(struct ata_interface *interface);

uint32_t marfs_get_max_lba(void);

uint8_t marfs_write_mbr(uint8_t *mbr);

struct marfs_superblock *marfs_read_superblock();

uint8_t marfs_write_superblock(struct marfs_superblock *sb);

uint32_t marfs_get_free_lba_block(void);

uint8_t marfs_mark_block_as_used(uint32_t lba_sector);

uint8_t marfs_mark_block_as_free(uint32_t lba_sector);

uint32_t marfs_get_free_lba_inode(void);

void marfs_mark_inode_as_free(uint32_t lba_sector);

// marfs_disklevel.c
void marfs_format(void);

// marfs_new_file.c
uint32_t marfs_new_file(uint64_t size, uint8_t *data, uint32_t uid, uint8_t exec, uint8_t dir);

// marfs_dir.c
uint32_t marfs_new_dir(uint32_t uid);

void marfs_add_to_dir(uint32_t lba_inode, char *filename, uint32_t lba);

// marfs_read_whole_file.c
uint32_t marfs_get_block(struct marfs_inode *inode, uint32_t i);

void marfs_read_whole_file(uint32_t lba_inode, uint8_t *buffer);

uint8_t *marfs_allocate_and_read_whole_file(uint32_t lba_inode);

#endif
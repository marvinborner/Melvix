#include <stdint.h>
#include <kernel/fs/ata_pio.h>
#include <kernel/fs/marfs/marfs.h>

void marfs_format(void)
{
	// Create superblock
	struct marfs_superblock sb;
	sb.signature = 0x1083B99F34B59645; // Huh, magic?!
	sb.n_inodes = (marfs_get_max_lba() - 2) >> 5;
	sb.n_chunks = (marfs_get_max_lba() - (2 + sb.n_inodes)) >> 9;
	sb.n_first_unallocated_inode = 0;
	sb.s_first_inode = 2;
	sb.s_first_chunk = 2 + sb.n_inodes;

	// Write to disk
	marfs_write_superblock(&sb);

	// Initialize the inodes
	for (uint32_t i = 0; i < sb.n_inodes; i++)
		ata_clear28(interface, 2 + i);

	// Initialize the chunks
	for (uint32_t i = 0; i < sb.n_chunks; i++)
		ata_clear28(interface, sb.s_first_chunk + i * 512);
}

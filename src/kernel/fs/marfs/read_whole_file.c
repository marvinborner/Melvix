#include <stdint.h>
#include <kernel/fs/ata_pio.h>
#include <kernel/fs/marfs/marfs.h>
#include <kernel/memory/kheap.h>

static uint8_t last_max_level = 0;

uint32_t marfs_get_recursive(uint8_t level, uint32_t i, uint32_t rec_lba)
{
    if (level > last_max_level) last_max_level = level;
    uint32_t *contents = (uint32_t *) ata_read28(interface, rec_lba);
    uint32_t idx = i - 10;
    if (last_max_level > 1) idx -= 1 << 7;
    if (last_max_level > 2) idx -= 1 << (7 * 2);
    if (last_max_level > 3) idx -= 1 << (7 * 3);
    idx >>= 7 * (level - 1);

    uint32_t next_rec_lba = contents[idx];
    kfree(contents);

    uint32_t toRet;
    if (level > 1) toRet = marfs_get_recursive((uint8_t) (level - 1), i, next_rec_lba);
    else toRet = next_rec_lba;
    last_max_level = 0;
    return toRet;
}

uint32_t marfs_get_block(struct marfs_inode *inode, uint32_t i)
{
    if (i > 9 + (128 * 128 * 128)) {
        return marfs_get_recursive(4, i, inode->ext_4);
    } else if (i > 9 + (128 * 128)) {
        return marfs_get_recursive(3, i, inode->ext_3);
    } else if (i > 9 + 128) {
        return marfs_get_recursive(2, i, inode->ext_2);
    } else if (i > 9) {
        return marfs_get_recursive(1, i, inode->ext_1);
    } else {
        return inode->DBPs[i];
    }
}

void marfs_read_whole_file(uint32_t lba_inode, uint8_t *buffer)
{
    struct marfs_inode *inode = (struct marfs_inode *) ata_read28(interface, lba_inode);

    uint32_t size_in_blocks = inode->n_blocks;
    for (uint32_t i = 0; i < size_in_blocks; i++) {
        uint32_t this_block = marfs_get_block(inode, i);
        uint8_t *this_block_contents = ata_read28(interface, this_block);
        uint16_t upper_bound = (uint16_t) ((i != size_in_blocks - 1) ? 512 : (inode->size % 512));
        for (uint16_t j = 0; j < upper_bound; j++) buffer[(i * 512) + j] = this_block_contents[j];
        kfree(this_block_contents);
    }

    kfree(inode);
}

// TODO: Beautify
uint8_t *marfs_allocate_and_read_whole_file(uint32_t lba_inode)
{
    struct marfs_inode *inode = (struct marfs_inode *) ata_read28(interface, lba_inode);
    uint64_t size = inode->size;
    kfree(inode);

    uint8_t *buffer = (uint8_t *) kmalloc((uint32_t) size);
    marfs_read_whole_file(lba_inode, buffer);
    return buffer;
}

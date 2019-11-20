#include <stdint.h>
#include <kernel/fs/ata_pio.h>
#include <kernel/lib/alloc.h>
#include "marfs.h"

static uint8_t last_maxlevel = 0;

uint32_t marfs_get_recursive(uint8_t level, uint32_t i, uint32_t recLBA) {
    if (level > last_maxlevel) last_maxlevel = level;
    uint32_t *contents = (uint32_t *) ATA_read28(iface, recLBA);
    uint32_t idx = i - 10;
    if (last_maxlevel > 1) idx -= 1 << 7;
    if (last_maxlevel > 2) idx -= 1 << (7 * 2);
    if (last_maxlevel > 3) idx -= 1 << (7 * 3);
    idx >>= 7 * (level - 1);

    uint32_t next_recLBA = contents[idx];
    kfree(contents);

    uint32_t toRet;
    if (level > 1) toRet = marfs_get_recursive(level - 1, i, next_recLBA);
    else toRet = next_recLBA;
    last_maxlevel = 0;
    return toRet;
}

uint32_t marfs_get_block(struct marfs_INODE *inode, uint32_t i) {
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

void marfs_read_whole_file(uint32_t LBAinode, uint8_t *buffer) {
    struct marfs_INODE *inode = (struct marfs_INODE *) ATA_read28(iface, LBAinode);

    uint32_t size_in_blocks = inode->n_blocks;
    for (uint32_t i = 0; i < size_in_blocks; i++) {
        uint32_t this_block = marfs_get_block(inode, i);
        uint8_t *this_block_contents = ATA_read28(iface, this_block);
        uint16_t upper_bound = (i != size_in_blocks - 1) ? 512 : (inode->size % 512);
        for (uint16_t j = 0; j < upper_bound; j++) buffer[(i * 512) + j] = this_block_contents[j];
        kfree(this_block_contents);
    }

    kfree(inode);
}

// TODO: Beautify
uint8_t *marfs_allocate_and_read_whole_file(uint32_t LBAinode) {
    struct marfs_INODE *inode = (struct marfs_INODE *) ATA_read28(iface, LBAinode);
    uint64_t size = inode->size;
    kfree(inode);

    uint8_t *buffer = kmalloc(size);
    marfs_read_whole_file(LBAinode, buffer);
    return buffer;
}

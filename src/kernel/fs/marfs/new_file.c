#include <stdint.h>
#include <kernel/fs/ata_pio.h>
#include <kernel/lib/alloc.h>
#include "marfs.h"

static uint8_t last_maxlevel = 0;

void marfs_update_recursive(uint8_t level, uint32_t i, uint32_t recLBA, uint32_t realLBA) {
    if (level > last_maxlevel) last_maxlevel = level;
    uint32_t *contents = (uint32_t *) ATA_read28(iface, recLBA);

    uint32_t idx = i - 10;
    if (last_maxlevel > 1) idx -= 1 << 7;
    if (last_maxlevel > 2) idx -= 1 << (7 * 2);
    if (last_maxlevel > 3) idx -= 1 << (7 * 3);
    idx >>= 7 * (level - 1);

    if (level > 1) {
        if (!contents[idx]) {
            contents[idx] = marfs_get_free_lba_block();
            marfs_mark_block_as_used(contents[idx]);
        }
    } else {
        contents[idx] = realLBA;
    }

    ATA_write28(iface, recLBA, (uint8_t *) contents);

    uint32_t contents_idx = contents[idx];
    kfree(contents);
    if (level != 1) {
        marfs_update_recursive(level - 1, i, contents_idx, realLBA);
    }
    last_maxlevel = 0;
}

uint32_t marfs_new_file(uint64_t size, uint8_t *data, uint32_t uid, uint8_t exec, uint8_t dir) {
    struct marfs_INODE *inode = (struct marfs_INODE *) kcalloc(1, 512);
    inode->size = size;
    inode->creation_time = inode->last_mod_time = inode->last_access_time = 0; // TODO: POSIX time
    inode->n_blocks = size / 512;
    if (size % 512) inode->n_blocks++;
    inode->uid = uid;
    inode->isApp = exec;
    inode->isDir = dir;
    inode->isUsed = 1;

    uint32_t size_in_blocks = inode->n_blocks;

    uint32_t LBA_singly, LBA_doubly, LBA_triply, LBA_quadruply;
    LBA_singly = LBA_doubly = LBA_triply = LBA_quadruply = 0;
    for (uint32_t i = 0; i < size_in_blocks; i++) {
        uint32_t thisblock = marfs_get_free_lba_block();
        if (i != size_in_blocks - 1) {
            ATA_write28(iface, thisblock, data);
        } else if (size % 512) {
            uint8_t contents[512] = {0};
            for (uint16_t i = 0; i < size % 512; i++) contents[i] = data[i];
            ATA_write28(iface, thisblock, contents);
        }
        data += 512;
        marfs_mark_block_as_used(thisblock);

        if (i > 9 + (128 * 128 * 128)) {
            if (!LBA_quadruply) {
                LBA_quadruply = marfs_get_free_lba_block();
                marfs_mark_block_as_used(LBA_quadruply);
                inode->ext_4 = LBA_quadruply;
            }
            marfs_update_recursive(4, i, LBA_quadruply, thisblock);
        } else if (i > 9 + (128 * 128)) {
            if (!LBA_triply) {
                LBA_triply = marfs_get_free_lba_block();
                marfs_mark_block_as_used(LBA_triply);
                inode->ext_3 = LBA_triply;
            }
            marfs_update_recursive(3, i, LBA_triply, thisblock);
        } else if (i > 9 + 128) {
            if (!LBA_doubly) {
                LBA_doubly = marfs_get_free_lba_block();
                marfs_mark_block_as_used(LBA_doubly);
                inode->ext_2 = LBA_doubly;
            }
            marfs_update_recursive(2, i, LBA_doubly, thisblock);
        } else if (i > 9) {
            if (!LBA_singly) {
                LBA_singly = marfs_get_free_lba_block();
                marfs_mark_block_as_used(LBA_singly);
                inode->ext_1 = LBA_singly;
            }
            marfs_update_recursive(1, i, LBA_singly, thisblock);
        } else {
            inode->DBPs[i] = thisblock;
        }
    }

    // Write the inode
    uint32_t inode_LBA = marfs_get_free_lba_inode();
    ATA_write28(iface, inode_LBA, (uint8_t *) inode);

    return inode_LBA;
}
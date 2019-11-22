#include <stdint.h>
#include <mlibc/stdlib.h>
#include <kernel/fs/ata_pio.h>
#include <kernel/fs/marfs/marfs.h>

uint8_t marfs_init(struct ATA_INTERFACE *_iface) {
    iface = _iface;
    uint16_t identifydata[256 * 2];
    uint8_t ret = ATA_identify(iface, identifydata);
    maxLBA = (identifydata[61] << 16) + identifydata[60];
    return ret;
}

uint32_t marfs_get_max_lba(void) { return maxLBA; }

uint8_t marfs_write_mbr(uint8_t *mbr) {
    return ATA_write28(iface, 0, mbr);
}

struct marfs_SUPERBLOCK *marfs_read_superblock() {
    struct marfs_SUPERBLOCK *p = (struct marfs_SUPERBLOCK *) ATA_read28(iface, 1);
    sb_cache = *p;
    return p;
}

uint8_t marfs_writeSB(struct marfs_SUPERBLOCK *sb) {
    sb_cache = *sb;
    return ATA_write28(iface, 1, (uint8_t *) sb);
}

uint32_t marfs_get_free_lba_block(void) {
    uint32_t offset = 2 + sb_cache.s_first_chunk;
    uint8_t *p = 0;
    for (uint32_t i = 0; i < sb_cache.n_chunks; i++) {
        p = ATA_read28(iface, offset);
        if (!(*p & 0x80)) break;
        kfree(p);
        offset += 512;
    }

    offset++;
    for (uint16_t i = 1; i < 512; i++) {
        if (!p[i]) break;
        offset++;
    }
    kfree(p);

    ATA_clear28(iface, offset);

    return offset;
}

static uint8_t marfs_mark_block(uint32_t lba_sector, uint8_t mode) {
    lba_sector -= 2;
    lba_sector -= sb_cache.s_first_chunk;
    uint16_t block_in_chunk = lba_sector % 512;
    lba_sector /= 512;
    lba_sector = 2 + sb_cache.s_first_chunk + (512 * lba_sector);

    uint8_t *p = ATA_read28(iface, lba_sector);
    p[block_in_chunk] = mode;

    if (mode == 0) {
        p[0] = 0;
    } else {
        uint8_t full_chunk = 1;
        for (uint16_t i = 1; i < 512; i++) {
            if (!p[i]) {
                full_chunk = 0;
                break;
            }
        }
        p[0] = full_chunk;
    }

    uint8_t ret = ATA_write28(iface, lba_sector, p);
    kfree(p);
    return ret;
}

uint8_t marfs_mark_block_as_free(uint32_t lba_sector) { return marfs_mark_block(lba_sector, 0); }

uint8_t marfs_mark_block_as_used(uint32_t lba_sector) { return marfs_mark_block(lba_sector, 1); }

uint32_t marfs_get_free_lba_inode(void) {
    uint32_t offset;
    for (offset = 2; offset < 2 + sb_cache.n_inodes; offset++) {
        struct marfs_INODE *inode = (struct marfs_INODE *) ATA_read28(iface, offset);
        uint8_t used = inode->isUsed;
        kfree(inode);
        if (!used) break;
    }
    return offset;
}

void marfs_mark_inode_as_free(uint32_t lba_sector) {
    struct marfs_INODE *inode = (struct marfs_INODE *) ATA_read28(iface, lba_sector);
    inode->isUsed = 0;
    ATA_write28(iface, lba_sector, (uint8_t *) inode);
    kfree(inode);
}
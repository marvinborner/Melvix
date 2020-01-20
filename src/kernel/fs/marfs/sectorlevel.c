#include <stdint.h>
#include <kernel/fs/ata_pio.h>
#include <kernel/fs/marfs/marfs.h>
#include <kernel/memory/kheap.h>

uint8_t marfs_init(struct ata_interface *_interface)
{
    interface = _interface;
    uint16_t identify_data[256 * 2];
    uint8_t ret = ata_identify(interface, identify_data);
    max_lba = (identify_data[61] << 16) + identify_data[60];
    return ret;
}

uint32_t marfs_get_max_lba(void)
{
    return max_lba;
}

uint8_t marfs_write_mbr(uint8_t *mbr)
{
    return ata_write28(interface, 0, mbr);
}

struct marfs_superblock *marfs_read_superblock()
{
    struct marfs_superblock *p = (struct marfs_superblock *) ata_read28(interface, 1);
    sb_cache = *p;
    return p;
}

uint8_t marfs_write_superblock(struct marfs_superblock *sb)
{
    sb_cache = *sb;
    return ata_write28(interface, 1, (uint8_t *) sb);
}

uint32_t marfs_get_free_lba_block(void)
{
    uint32_t offset = 2 + sb_cache.s_first_chunk;
    uint8_t *p = 0;
    for (uint32_t i = 0; i < sb_cache.n_chunks; i++) {
        p = ata_read28(interface, offset);
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

    ata_clear28(interface, offset);

    return offset;
}

static uint8_t marfs_mark_block(uint32_t lba_sector, uint8_t mode)
{
    lba_sector -= 2;
    lba_sector -= sb_cache.s_first_chunk;
    uint16_t block_in_chunk = (uint16_t) (lba_sector % 512);
    lba_sector /= 512;
    lba_sector = 2 + sb_cache.s_first_chunk + (512 * lba_sector);

    uint8_t *p = ata_read28(interface, lba_sector);
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

    uint8_t ret = ata_write28(interface, lba_sector, p);
    kfree(p);
    return ret;
}

uint8_t marfs_mark_block_as_free(uint32_t lba_sector)
{
    return marfs_mark_block(lba_sector, 0);
}

uint8_t marfs_mark_block_as_used(uint32_t lba_sector)
{
    return marfs_mark_block(lba_sector, 1);
}

uint32_t marfs_get_free_lba_inode(void)
{
    uint32_t offset;
    for (offset = 2; offset < 2 + sb_cache.n_inodes; offset++) {
        struct marfs_inode *inode = (struct marfs_inode *) ata_read28(interface, offset);
        uint8_t used = inode->is_used;
        kfree(inode);
        if (!used) break;
    }
    return offset;
}

void marfs_mark_inode_as_free(uint32_t lba_sector)
{
    struct marfs_inode *inode = (struct marfs_inode *) ata_read28(interface, lba_sector);
    inode->is_used = 0;
    ata_write28(interface, lba_sector, (uint8_t *) inode);
    kfree(inode);
}
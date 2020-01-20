#include <stdint.h>
#include <kernel/fs/ata_pio.h>
#include <kernel/fs/marfs/marfs.h>
#include <kernel/memory/kheap.h>

static uint8_t last_max_level = 0;

void marfs_update_recursive(uint8_t level, uint32_t i, uint32_t rec_lba, uint32_t real_lba)
{
    if (level > last_max_level) last_max_level = level;
    uint32_t *contents = (uint32_t *) ata_read28(interface, rec_lba);

    uint32_t idx = i - 10;
    if (last_max_level > 1) idx -= 1 << 7;
    if (last_max_level > 2) idx -= 1 << (7 * 2);
    if (last_max_level > 3) idx -= 1 << (7 * 3);
    idx >>= 7 * (level - 1);

    if (level > 1) {
        if (!contents[idx]) {
            contents[idx] = marfs_get_free_lba_block();
            marfs_mark_block_as_used(contents[idx]);
        }
    } else {
        contents[idx] = real_lba;
    }

    ata_write28(interface, rec_lba, (uint8_t *) contents);

    uint32_t contents_idx = contents[idx];
    kfree(contents);
    if (level != 1) {
        marfs_update_recursive((uint8_t) (level - 1), i, contents_idx, real_lba);
    }
    last_max_level = 0;
}

uint32_t marfs_new_file(uint64_t size, uint8_t *data, uint32_t uid, uint8_t exec, uint8_t dir)
{
    struct marfs_inode *inode = (struct marfs_inode *) kmalloc(512);
    inode->size = size;
    inode->creation_time = inode->last_mod_time = inode->last_access_time = 0; // TODO: POSIX time
    inode->n_blocks = (uint32_t) (size / 512);
    if (size % 512) inode->n_blocks++;
    inode->uid = uid;
    inode->is_app = exec;
    inode->is_dir = dir;
    inode->is_used = 1;

    uint32_t size_in_blocks = inode->n_blocks;

    uint32_t lba_singly, lba_doubly, lba_triply, lba_quadruply;
    lba_singly = lba_doubly = lba_triply = lba_quadruply = 0;
    for (uint32_t i = 0; i < size_in_blocks; i++) {
        uint32_t this_block = marfs_get_free_lba_block();
        if (i != size_in_blocks - 1) {
            ata_write28(interface, this_block, data);
        } else if (size % 512) {
            uint8_t contents[512] = {0};
            for (uint16_t i = 0; i < size % 512; i++) contents[i] = data[i];
            ata_write28(interface, this_block, contents);
        }
        data += 512;
        marfs_mark_block_as_used(this_block);

        if (i > 9 + (128 * 128 * 128)) {
            if (!lba_quadruply) {
                lba_quadruply = marfs_get_free_lba_block();
                marfs_mark_block_as_used(lba_quadruply);
                inode->ext_4 = lba_quadruply;
            }
            marfs_update_recursive(4, i, lba_quadruply, this_block);
        } else if (i > 9 + (128 * 128)) {
            if (!lba_triply) {
                lba_triply = marfs_get_free_lba_block();
                marfs_mark_block_as_used(lba_triply);
                inode->ext_3 = lba_triply;
            }
            marfs_update_recursive(3, i, lba_triply, this_block);
        } else if (i > 9 + 128) {
            if (!lba_doubly) {
                lba_doubly = marfs_get_free_lba_block();
                marfs_mark_block_as_used(lba_doubly);
                inode->ext_2 = lba_doubly;
            }
            marfs_update_recursive(2, i, lba_doubly, this_block);
        } else if (i > 9) {
            if (!lba_singly) {
                lba_singly = marfs_get_free_lba_block();
                marfs_mark_block_as_used(lba_singly);
                inode->ext_1 = lba_singly;
            }
            marfs_update_recursive(1, i, lba_singly, this_block);
        } else {
            inode->DBPs[i] = this_block;
        }
    }

    // Write the inode
    uint32_t inode_lba = marfs_get_free_lba_inode();
    ata_write28(interface, inode_lba, (uint8_t *) inode);

    return inode_lba;
}
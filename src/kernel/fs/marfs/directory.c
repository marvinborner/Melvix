#include <stdint.h>
#include <kernel/fs/ata_pio.h>
#include <kernel/lib/alloc.h>
#include <kernel/lib/lib.h>
#include "marfs.h"

uint32_t marfs_new_dir(uint32_t uid) { return marfs_new_file(0, 0, uid, 0, 1); }


void marfs_add_to_dir(uint32_t LBAinode, char *filename, uint32_t lba) {
    struct marfs_INODE *inode = (struct marfs_INODE *) ATA_read28(iface, LBAinode);

    // Read the content
    uint8_t *old = marfs_allocate_and_read_whole_file(LBAinode);

    // Allocate memory
    uint8_t *contents = kmalloc(inode->size + strlen(filename) + 1 + 4);

    // Copy the content
    uint8_t lastWasNull = 0;
    uint64_t newsize = 0;
    for (uint64_t i = 0; i < inode->size; i++) {
        if (old[i] == 0 && lastWasNull) continue;

        contents[newsize++] = old[i];
        lastWasNull = (old[i] == 0);
    }
    kfree(old);

    // Append new file
    for (uint16_t i = 0; i <= strlen(filename); i++) contents[newsize++] = filename[i];
    for (signed char j = 24; j > 0; j -= 8) contents[newsize++] = (lba >> j) & 0xFF;

    // Free the blocks
    uint32_t newsize_in_blocks = newsize / 512;
    if (newsize % 512) newsize_in_blocks++;
    for (uint32_t i = 0; i < newsize_in_blocks; i++)
        marfs_mark_block_as_free(marfs_get_block(inode, i));

    // Overwrite
    uint32_t aux_inode = marfs_new_file(newsize, contents, 0xDEADBEEF, 0, 0);
    struct marfs_INODE *real_aux_inode = (struct marfs_INODE *) ATA_read28(iface, aux_inode);
    for (uint8_t i = 0; i < 10; i++) inode->DBPs[i] = real_aux_inode->DBPs[i];
    inode->ext_1 = real_aux_inode->ext_1;
    inode->ext_2 = real_aux_inode->ext_2;
    inode->ext_3 = real_aux_inode->ext_3;
    inode->ext_4 = real_aux_inode->ext_4;
    real_aux_inode->isUsed = 0;
    ATA_write28(iface, aux_inode, (uint8_t *) real_aux_inode);
    kfree(real_aux_inode);

    inode->size = newsize;
    ATA_write28(iface, LBAinode, (uint8_t *) inode);
    kfree(inode);
}

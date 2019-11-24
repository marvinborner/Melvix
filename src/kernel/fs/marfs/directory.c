#include <stdint.h>
#include <kernel/fs/ata_pio.h>
#include <mlibc/stdlib.h>
#include <kernel/fs/marfs/marfs.h>

uint32_t marfs_new_dir(uint32_t uid)
{
    return marfs_new_file(0, 0, uid, 0, 1);
}


void marfs_add_to_dir(uint32_t lba_inode, char *filename, uint32_t lba)
{
    struct marfs_inode *inode = (struct marfs_inode *) ata_read28(interface, lba_inode);

    // Read the content
    uint8_t *old = marfs_allocate_and_read_whole_file(lba_inode);

    // Allocate memory
    uint8_t *contents = kmalloc(inode->size + strlen(filename) + 1 + 4);

    // Copy the content
    uint8_t last_was_null = 0;
    uint64_t new_size = 0;
    for (uint64_t i = 0; i < inode->size; i++) {
        if (old[i] == 0 && last_was_null) continue;

        contents[new_size++] = old[i];
        last_was_null = (old[i] == 0);
    }
    kfree(old);

    // Append new file
    for (size_t i = 0; i <= strlen(filename); i++) contents[new_size++] = filename[i];
    for (signed char j = 24; j > 0; j -= 8) contents[new_size++] = (lba >> j) & 0xFF;

    // Free the blocks
    uint32_t new_size_in_blocks = new_size / 512;
    if (new_size % 512) new_size_in_blocks++;
    for (uint32_t i = 0; i < new_size_in_blocks; i++)
        marfs_mark_block_as_free(marfs_get_block(inode, i));

    // Overwrite
    uint32_t aux_inode = marfs_new_file(new_size, contents, 0xDEADBEEF, 0, 0);
    struct marfs_inode *real_aux_inode = (struct marfs_inode *) ata_read28(interface, aux_inode);
    for (uint8_t i = 0; i < 10; i++) inode->DBPs[i] = real_aux_inode->DBPs[i];
    inode->ext_1 = real_aux_inode->ext_1;
    inode->ext_2 = real_aux_inode->ext_2;
    inode->ext_3 = real_aux_inode->ext_3;
    inode->ext_4 = real_aux_inode->ext_4;
    real_aux_inode->is_used = 0;
    ata_write28(interface, aux_inode, (uint8_t *) real_aux_inode);
    kfree(real_aux_inode);

    inode->size = new_size;
    ata_write28(interface, lba_inode, (uint8_t *) inode);
    kfree(inode);
}

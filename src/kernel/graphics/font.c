#include <kernel/fs/marfs/marfs.h>
#include <kernel/paging/paging.h>
#include <kernel/io/io.h>
#include <kernel/graphics/font.h>
#include <mlibc/stdlib/liballoc.h>
#include <kernel/fs/ata_pio.h>

void font_install() {
    uint8_t boot_drive_id = (uint8_t) (*((uint8_t *) 0x9000));
    if (boot_drive_id != 0xE0) {
        struct ata_interface *primary_master = new_ata(1, 0x1F0);
        marfs_init(primary_master);

        font = (struct font *) kmalloc(100000); // High quality shit
        marfs_read_whole_file(4, (uint8_t *) font);
    }
}
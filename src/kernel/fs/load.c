#include <kernel/fs/load.h>
#include <kernel/fs/marfs/marfs.h>
#include <kernel/fs/ata_pio.h>
#include <kernel/fs/atapi_pio.h>
#include <kernel/system.h>
#include <kernel/fs/iso9660/iso9660.h>
#include <kernel/memory/alloc.h>

void load_binaries()
{
    userspace = (uint32_t) kmalloc(10000);
    font = (struct font *) kmalloc(100000);; // High quality shit

    uint8_t boot_drive_id = (uint8_t) (*((uint8_t *) 0x9000));
    if (boot_drive_id != 0xE0) {
        struct ata_interface *primary_master = new_ata(1, 0x1F0);
        marfs_init(primary_master);
        marfs_read_whole_file(4, (uint8_t *) userspace);
        marfs_read_whole_file(5, (uint8_t *) font);
    } else {
        char *font_p[] = {"FONT.BIN"};
        struct iso9660_entity *font_e = ISO9660_get(font_p, 1);
        if (!font_e) panic("Font not found!");
        ATAPI_granular_read(1 + (font_e->length / 2048), font_e->lba, (uint8_t *) font);
        kfree(font_e);

        char *user_p[] = {"USER.BIN"};
        struct iso9660_entity *user_e = ISO9660_get(user_p, 1);
        if (!user_e) panic("Userspace binary not found!");
        ATAPI_granular_read(1 + (user_e->length / 2048), user_e->lba, (uint8_t *) userspace);
        kfree(user_e);
    }
    vga_log("Successfully loaded binaries");
}
#include <kernel/fs/marfs/marfs.h>
#include <kernel/paging/paging.h>
#include <kernel/graphics/font.h>
#include <kernel/lib/stdlib/liballoc.h>
#include <kernel/fs/ata_pio.h>
#include <kernel/fs/atapi_pio.h>
#include <kernel/system.h>
#include <kernel/fs/iso9660/iso9660.h>

void font_install()
{
    font = (struct font *) paging_alloc_pages(25);; // High quality shit
    paging_set_user((uint32_t) font, 25);

    uint8_t boot_drive_id = (uint8_t) (*((uint8_t *) 0x9000));
    if (boot_drive_id != 0xE0) {
        struct ata_interface *primary_master = new_ata(1, 0x1F0);
        marfs_init(primary_master);
        marfs_read_whole_file(5, (uint8_t *) font);
    } else {
        char *font_p[] = {"FONT.BIN"};
        struct iso9660_entity *font_e = ISO9660_get(font_p, 1);
        if (!font_e) panic("Font not found!");
        ATAPI_granular_read(1 + (font_e->length / 2048), font_e->lba, (uint8_t *) font);
        kfree(font_e);
    }
    vga_log("Successfully loaded font");
}
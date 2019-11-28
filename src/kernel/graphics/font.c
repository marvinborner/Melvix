#include <kernel/fs/marfs/marfs.h>
#include <kernel/paging/paging.h>
#include <kernel/io/io.h>
#include <mlibc/stdlib/liballoc.h>

void font_install() {
    uint8_t boot_drive_id = (uint8_t) (*((uint8_t *) 0x9000));
    if (boot_drive_id != 0xE0) {
        uint32_t *font = (uint32_t *) kmalloc(0x18326); // High quality shit
        marfs_read_whole_file(4, (uint8_t *) (font + 4096));

        for (int i = 0; i < 10; i++) {
            serial_write_hex(font[i]);
            serial_write("\n");
        }
    }
}
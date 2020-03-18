#include <kernel/graphics/vesa.h>
#include <kernel/fs/ata_pio.h>
#include <kernel/io/io.h>
#include <kernel/fs/marfs/marfs.h>
#include <kernel/fs/iso9660/iso9660.h>
#include <kernel/fs/atapi_pio.h>
#include <kernel/acpi/acpi.h>
#include <kernel/lib/stdio.h>
#include <kernel/timer/timer.h>
#include <kernel/memory/alloc.h>
#include <kernel/fs/load.h>

void install_melvix()
{
    info("You're booting from a CD, Melvix will only run after an installation");
    cli();
    struct ata_interface *primary_master = new_ata(1, 0x1F0);
    if (marfs_init(primary_master) != 0) {
        panic("No HDD found!");
    }

    struct marfs_superblock *currentSB = marfs_read_superblock();
    if (currentSB->signature == 0x1083B99F34B59645) { // WEEEOOOWEEEOOO
        panic("Melvix seems to be already installed!");
    }
    kfree(currentSB);

    info("Installing...\n");

    // Copy MBR
    info("Copying MBR... ");
    serial_printf("Copying MBR... ");
    char *stage1_p[] = {"BOOT", "HDD1.BIN"};
    struct iso9660_entity *stage1_e = ISO9660_get(stage1_p, 2);
    if (!stage1_e)
        panic("Couldn't find the first HDD bootloader!");
    uint8_t *stage1 = ISO9660_read(stage1_e);
    kfree(stage1_e);
    marfs_write_mbr(stage1);

    // Format disk
    info("Formatting disk...");
    serial_printf("Formatting disk...");
    marfs_format();

    // Copy second stage
    info("Copying second stage...");
    serial_printf("Copying second stage...");
    char *stage2_p[] = {"BOOT", "HDD2.BIN"};
    struct iso9660_entity *stage2_e = ISO9660_get(stage2_p, 2);
    if (!stage2_e)
        panic("Couldn't find the second HDD bootloader!");
    uint8_t *stage2 = ISO9660_read(stage2_e);
    marfs_new_file(stage2_e->length, stage2, 0, 0, 0);
    kfree(stage2_e);

    // Copy the kernel
    info("Copying the kernel...");
    serial_printf("Copying the kernel...");
    char *kernel_p[] = {"BOOT", "KERNEL.BIN"};
    struct iso9660_entity *kernel_e = ISO9660_get(kernel_p, 2);
    if (!kernel_e)
        panic("WTH Kernel not found!?");
    uint8_t *kernel = (uint8_t *) kmalloc(kernel_e->length + 2048);
    ATAPI_granular_read(1 + (kernel_e->length / 2048), kernel_e->lba, kernel);
    marfs_new_file(kernel_e->length, kernel, 0, 0, 0);
    kfree(kernel);
    kfree(kernel_e);

    // Copy the userspace binary
    info("Copying userspace... ");
    serial_printf("Copying userspace... ");
    char *userspace_p[] = {"USER.BIN"};
    struct iso9660_entity *userspace_e = ISO9660_get(userspace_p, 1);
    if (!userspace_e)
        panic("Userspace not found!");
    uint8_t *userspace = ISO9660_read(userspace_e);
    marfs_new_file(userspace_e->length, userspace, 0, 0, 0);
    kfree(userspace_e);

    // Copy the global font binary
    info("Copying font... ");
    serial_printf("Copying font... ");
    char *font_p[] = {"FONT.BIN"};
    struct iso9660_entity *font_e = ISO9660_get(font_p, 1);
    marfs_new_file(font_e->length, (uint8_t *) font, 0, 0, 0);
    kfree(font_e);

    info("Installation successful!");
    serial_printf("Installation successful! (in %d ticks)", get_time());
    // timer_wait(200);
    acpi_poweroff();
    halt_loop();
}

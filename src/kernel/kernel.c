#include <kernel/graphics/vesa.h>
#include <kernel/gdt/gdt.h>
#include <kernel/interrupts/interrupts.h>
#include <kernel/io/io.h>
#include <kernel/timer/timer.h>
#include <kernel/paging/paging.h>
#include <kernel/input/input.h>
#include <kernel/acpi/acpi.h>
#include <kernel/smbios/smbios.h>
#include <kernel/fs/install.h>
#include <kernel/lib/lib.h>
#include <kernel/syscall/syscall.h>
#include <kernel/fs/marfs/marfs.h>

extern void jump_userspace();

void kernel_main()
{
    vga_log("Installing basic features of Melvix...", 0);

    // Install features
    memory_init();
    gdt_install();
    init_serial();
    acpi_install();
    paging_install();
    idt_install();
    isrs_install();
    irq_install();
    font_install();
    set_optimal_resolution();

    // Install drivers
    asm ("cli");
    timer_install();
    mouse_install();
    keyboard_install();
    asm ("sti");

    // Get hardware information
    get_smbios();

    // Print total memory
    info("Total memory found: %dMiB", (memory_get_all() >> 10) + 1);

    uint8_t boot_drive_id = (uint8_t) (*((uint8_t *) 0x9000));
    if (boot_drive_id == 0xE0)
        install_melvix();

    // User mode!
    info("Switching to user mode...");
    syscalls_install();
    tss_flush();
    uint32_t userspace = paging_alloc_pages(10);
    paging_set_user(userspace, 10);
    marfs_read_whole_file(4, (uint8_t *) (userspace + 4096));
    jump_userspace(userspace + 4096);

    panic("This should NOT happen!");

    // asm ("div %0" :: "r"(0)); // Exception testing x/0
    loop:
    asm ("hlt");
    goto loop;
}

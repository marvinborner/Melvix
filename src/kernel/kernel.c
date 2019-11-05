#include <kernel/graphics/vesa.h>
#include <kernel/gdt/gdt.h>
#include <kernel/interrupts/interrupts.h>
#include <kernel/io/io.h>
#include <kernel/timer/timer.h>
#include <kernel/paging/paging.h>
#include <kernel/input/input.h>
#include <kernel/acpi/acpi.h>
#include <kernel/mutliboot.h>
#include <kernel/fs/initrd.h>
#include <kernel/syscall/syscall.h>
#include <kernel/smbios/smbios.h>

extern void switch_to_user();

void kernel_main(struct multiboot *mboot_ptr) {
    vga_log("Installing basic features of Melvix...", 0);
    // Install features
    timer_install();
    gdt_install();
    init_serial();
    paging_install();
    acpi_install();
    idt_install();
    isrs_install();
    irq_install();
    set_optimal_resolution();

    // Install drivers
    asm volatile ("cli");
    mouse_install();
    keyboard_install();
    asm volatile ("sti");

    // Get hardware information
    get_smbios();

    // Setup initial ramdisk
    assert(mboot_ptr->mods_count > 0);
    uint32_t initrd_location = *((uint32_t *) mboot_ptr->mods_addr);
    uint32_t initrd_end = *(uint32_t *) (mboot_ptr->mods_addr + 4);
    paging_set_used(0, (initrd_end >> 12) + 1);
    fs_root = initialise_initrd(initrd_location);
    initrd_test();

    // User mode!
    /* COMMENTED FOR DEVELOPMENT OF KERNEL
    info("Switching to user mode...");
    syscalls_install();
    switch_to_user();

    panic("This should NOT happen!");
    */

    // asm volatile  ("div %0" :: "r"(0)); // Exception testing x/0
    loop:
    asm volatile ("hlt");
    goto loop;
}

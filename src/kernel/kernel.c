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

void init() {
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
    keyboard_install();
    asm volatile ("sti");
}

void kernel_main(struct multiboot *mboot_ptr) {
    init();

    info("Kernel size in KiB: ");
    vesa_draw_number((int) end >> 10);

    assert(mboot_ptr->mods_count > 0);
    uint32_t initrd_location = *((uint32_t *) mboot_ptr->mods_addr);
    uint32_t initrd_end = *(uint32_t *) (mboot_ptr->mods_addr + 4);
    // Don't trample our module with placement accesses, please!
    // placement_address = initrd_end;
    fs_root = initialise_initrd(initrd_location);

    // asm volatile  ("div %0" :: "r"(0)); // Exception testing x/0
    loop:
    asm volatile ("hlt");
    goto loop;
}

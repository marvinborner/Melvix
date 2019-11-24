#include <kernel/graphics/vesa.h>
#include <kernel/gdt/gdt.h>
#include <kernel/interrupts/interrupts.h>
#include <kernel/io/io.h>
#include <kernel/timer/timer.h>
#include <kernel/paging/paging.h>
#include <kernel/input/input.h>
#include <kernel/acpi/acpi.h>
#include <kernel/syscall/syscall.h>
#include <kernel/smbios/smbios.h>
#include <kernel/fs/install.h>
#include <kernel/lib/lib.h>

extern void switch_to_user();

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
    set_optimal_resolution();

    // Install drivers
    asm ("cli");
    mouse_install();
    keyboard_install();
    timer_install();
    asm ("sti");

    // Get hardware information
    get_smbios();

    // Booting process complete - emulate newline key
    vesa_keyboard_char('\n');

    uint8_t boot_drive_id = (uint8_t) (*((uint8_t *) 0x9000));
    if (boot_drive_id == 0xE0) {
        install_melvix();
    }

    // User mode!
    /* COMMENTED FOR DEVELOPMENT OF KERNEL
    info("Switching to user mode...");
    syscalls_install();
    switch_to_user();

    panic("This should NOT happen!");
    */

    // asm ("div %0" :: "r"(0)); // Exception testing x/0
    loop:
    asm ("hlt");
    goto loop;
}

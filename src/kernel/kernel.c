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

    assert(mboot_ptr->mods_count > 0);
    uint32_t initrd_location = *((uint32_t *) mboot_ptr->mods_addr);
    uint32_t initrd_end = *(uint32_t *) (mboot_ptr->mods_addr + 4);
    paging_set_used(0, (initrd_end >> 12) + 1);

    fs_root = initialise_initrd(initrd_location);

    int i = 0;
    struct dirent *node = 0;
    vesa_draw_string("\n");
    while ((node = readdir_fs(fs_root, i)) != 0) {
        vesa_draw_string("Found file: ");
        vesa_draw_string(node->name);
        vesa_draw_string("\n");
        fs_node_t *fsnode = finddir_fs(fs_root, node->name);

        if ((fsnode->flags & 0x7) == FS_DIRECTORY)
            vesa_draw_string("\t (directory)\n");
        else {
            vesa_draw_string("\t contents: \"");
            uint8_t buf[fsnode->length];
            uint32_t sz = read_fs(fsnode, 0, fsnode->length, buf);
            for (uint32_t j = 0; j < sz; j++)
                vesa_draw_char(buf[j]);

            vesa_draw_string("\"\n");
        }
        i++;
    }

    // asm volatile  ("div %0" :: "r"(0)); // Exception testing x/0
    loop:
    asm volatile ("hlt");
    goto loop;
}

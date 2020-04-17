#include <kernel/multiboot.h>
#include <kernel/graphics/vesa.h>
#include <kernel/gdt/gdt.h>
#include <kernel/interrupts/interrupts.h>
#include <kernel/io/io.h>
#include <kernel/timer/timer.h>
#include <kernel/memory/paging.h>
#include <kernel/input/input.h>
#include <kernel/acpi/acpi.h>
#include <kernel/lib/lib.h>
#include <kernel/syscall/syscall.h>
#include <kernel/pci/pci.h>
#include <kernel/net/network.h>
#include <kernel/tasks/task.h>
#include <kernel/fs/load.h>
#include <kernel/fs/elf.h>
#include <kernel/lib/stdio.h>
#include <kernel/fs/ata.h>
#include <kernel/fs/ext2.h>
#include <kernel/fs/vfs.h>

void kernel_main(uint32_t magic, uint32_t multiboot_address)
{
	if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
		vga_log("Invalid boot magic!");
		halt_loop();
	}

	if (multiboot_address & 7) {
		vga_log("Unaligned mbi!");
		halt_loop();
	}

	vga_log("Installing basic features of Melvix...");

	// Install features
	gdt_install();
	init_serial();
	acpi_install();
	idt_install();
	isrs_install();
	irq_install();

	multiboot_parse(multiboot_address);
	paging_install();

	memory_print();

	vfs_init();
	ata_init();
	ext2_init("/dev/hda", "/");

	load_binaries();
	set_optimal_resolution();

	// Install drivers
	cli();
	timer_install();
	mouse_install();
	keyboard_install();
	pci_remap();
	network_install();
	sti();

	// tasking_install();

	// Get hardware information
	// get_smbios();

#ifdef INSTALL_MELVIX
	panic("Installation isn't supported right now!");
#endif

	// load_elf((char *)userspace);

	// syscalls_install();
	// exec(userspace);

	// panic("This should NOT happen!");

	// asm ("div %0" :: "r"(0)); // Exception testing x/0
}

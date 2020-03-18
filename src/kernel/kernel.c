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

void kernel_main(uint32_t initial_stack)
{
	initial_esp = initial_stack;
	vga_log("Installing basic features of Melvix...");

	// Install features
	memory_init();
	gdt_install();
	init_serial();
	acpi_install();
	idt_install();
	isrs_install();
	irq_install();
	paging_install();

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

	// Print total memory
	info("Total memory found: %dMiB", (memory_get_all() >> 10) + 1);

#ifdef INSTALL_MELVIX
#include <kernel/fs/install.h>
	serial_printf("Install flag given!");
	if ((*((uint8_t *)0x9000)) == 0xE0)
		install_melvix();
#endif

	loader_init();
	elf_init();
	exec_start((uint8_t *)userspace);

	//    syscalls_install();
	//    exec(userspace);

	panic("This should NOT happen!");

	// asm ("div %0" :: "r"(0)); // Exception testing x/0
}

#include <multiboot.h>
#include <graphics/vesa.h>
#include <gdt/gdt.h>
#include <interrupts/interrupts.h>
#include <io/io.h>
#include <timer/timer.h>
#include <memory/paging.h>
#include <input/input.h>
#include <acpi/acpi.h>
#include <lib/lib.h>
#include <lib/stdlib.h>
#include <syscall/syscall.h>
#include <pci/pci.h>
#include <net/network.h>
#include <fs/load.h>
#include <fs/elf.h>
#include <lib/stdio.h>
#include <fs/ata.h>
#include <fs/ext2.h>
#include <cmos/rtc.h>
#include <memory/alloc.h>

u32 stack_hold;

void kernel_main(u32 magic, u32 multiboot_address, u32 esp)
{
	stack_hold = esp;

	if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
		warn("Invalid boot magic!");
		halt_loop();
	}

	if (multiboot_address & 7) {
		warn("Unaligned mbi!");
		halt_loop();
	}

	info("Installing basic features of Melvix...");

	// Install features
	gdt_install();
	init_serial();
	idt_install();
	isrs_install();
	irq_install();

	multiboot_parse(multiboot_address);
	paging_install();

	// Install drivers
	cli();
	timer_install();
	mouse_install();
	keyboard_install();
	pci_remap();
	network_install();
	sti();

	memory_print();
	rtc_print();

	ata_init();
	ext2_init_fs();

	load_binaries();
	set_optimal_resolution();
	log("Content of /etc/test: %s", read_file("/etc/test"));

	syscalls_install();
	kexec("/bin/init");

	halt_loop();
	// asm ("div %0" :: "r"(0)); // Exception testing x/0
}
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
#include <kernel/lib/stdlib.h>
#include <kernel/syscall/syscall.h>
#include <kernel/pci/pci.h>
#include <kernel/net/network.h>
#include <kernel/fs/load.h>
#include <kernel/fs/elf.h>
#include <kernel/lib/stdio.h>
#include <kernel/fs/ata.h>
#include <kernel/fs/ext2.h>
#include <kernel/fs/vfs.h>
#include <kernel/cmos/rtc.h>
#include <kernel/memory/alloc.h>

uint32_t stack_hold;

void kernel_main(uint32_t magic, uint32_t multiboot_address, uint32_t esp)
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

	struct fs_node *test = (struct fs_node *)kmalloc(sizeof(struct fs_node));
	strcpy(test->name, "/etc/test");
	ext2_root->open(test);
	uint32_t size = ((struct ext2_file *)test->impl)->inode.size;
	char buf[size];
	ext2_root->read(test, 0, size, buf);
	buf[size - 1] = '\0';
	log("Content of /etc/test: %s", buf);
	ext2_root->close(test);

	syscalls_install();
	kexec("/bin/init");

	halt_loop();
	// asm ("div %0" :: "r"(0)); // Exception testing x/0
}
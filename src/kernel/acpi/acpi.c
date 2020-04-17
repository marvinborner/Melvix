// Important specification: https://uefi.org/sites/default/files/resources/ACPI_6_2.pdf
// HPET: https://www.intel.com/content/dam/www/public/us/en/documents/technical-specifications/software-developers-hpet-spec-1-0a.pdf

#include <stddef.h>
#include <kernel/system.h>
#include <kernel/multiboot.h>
#include <kernel/io/io.h>
#include <kernel/lib/lib.h>
#include <kernel/lib/stdlib.h>
#include <kernel/lib/stdio.h>
#include <kernel/acpi/acpi.h>
#include <kernel/memory/paging.h>

struct rsdt *rsdt;
struct fadt *fadt;
struct hpet *hpet;
struct apic *apic;

void acpi_init(struct rsdp *rsdp)
{
	// TODO: Fix usage of ACPI tables after paging!
	if (strncmp(rsdp->signature, "RSD PTR ", 8) == 0) {
		rsdt = (struct rsdt *)rsdp->rsdt_address;
		int entries = (rsdt->header.length - sizeof(rsdt->header)) / 4;

		for (int i = 0; i < entries; i++) {
			struct sdt_header *header = (struct sdt_header *)rsdt->sdt_pointer[i];
			if (strncmp(header->signature, "FACP", 4) == 0) {
				info("Found FADT");
				fadt = (struct fadt *)header;
			} else if (strncmp(header->signature, "HPET", 4) == 0) {
				info("Found HPET");
				hpet = (struct hpet *)header;
			} else if (strncmp(header->signature, "APIC", 4) == 0) {
				info("Found MADT");
				apic = (struct apic *)header;
			}
		}
	} else {
		warn("Wrong RSD signature!");
	}
}

void acpi_old_init(struct multiboot_tag_old_acpi *tag)
{
	acpi_init((struct rsdp *)tag->rsdp);
}

void acpi_new_init(struct multiboot_tag_new_acpi *tag)
{
	acpi_init((struct rsdp *)tag->rsdp);
}

void acpi_poweroff()
{
	cli();
	/*
	if (SCI_EN == 0) {
		warn("ACPI shutdown is not supported");
		return;
	}

	// Send shutdown command
	outw((uint16_t)(unsigned int)PM1a_CNT, (uint16_t)(SLP_TYPa | SLP_EN));
	if (PM1b_CNT != 0)
		outw((uint16_t)(unsigned int)PM1b_CNT, (uint16_t)(SLP_TYPb | SLP_EN));
	else {
		outw(0xB004, 0x2000); // Bochs
		outw(0x604, 0x2000); // QEMU
		outw(0x4004, 0x3400); // VirtualBox
	}
	*/
}

void reboot()
{
	cli();
	outb(fadt->reset_reg.address, fadt->reset_value);
	halt_loop();

	/* else?
	uint8_t good = 0x02;
	while (good & 0x02)
		good = inb(0x64);
	outb(0x64, 0xFE);
	halt_loop();
	*/
}

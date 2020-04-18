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
#include <kernel/memory/alloc.h>

struct rsdt *rsdt;
struct fadt *fadt;
struct hpet *hpet;
struct madt *madt;

int check_sum(struct sdt_header *header)
{
	uint8_t sum = 0;

	for (uint32_t i = 0; i < header->length; i++)
		sum += ((char *)header)[i];

	return sum == 0;
}

void acpi_init(struct rsdp *rsdp)
{
	struct sdt_header *header = (struct sdt_header *)kmalloc(sizeof(struct sdt_header));
	rsdt = (struct rsdt *)kmalloc(sizeof(struct rsdt));
	fadt = (struct fadt *)kmalloc(sizeof(struct fadt));
	hpet = (struct hpet *)kmalloc(sizeof(struct hpet));
	madt = (struct madt *)kmalloc(sizeof(struct madt));

	if (strncmp(rsdp->signature, "RSD PTR ", 8) == 0) {
		memcpy(rsdt, rsdp->rsdt_address, sizeof(struct rsdt) + 32);
		if (!check_sum((struct sdt_header *)rsdt)) {
			warn("Corrupted RSDT!");
		} else {
			uint32_t *pointer = (uint32_t *)(rsdt + 1);
			uint32_t *end = (uint32_t *)((uint8_t *)rsdt + rsdt->header.length);

			while (pointer < end) {
				uint32_t address = *pointer++;
				memcpy(header, (void *)address, sizeof(struct sdt_header));

				if (strncmp(header->signature, "FACP", 4) == 0) {
					info("Found FADT");
					memcpy(fadt, (void *)address, sizeof(struct fadt));
					if (!check_sum((struct sdt_header *)fadt))
						warn("Corrupted FADT!");
				} else if (strncmp(header->signature, "HPET", 4) == 0) {
					info("Found HPET");
					memcpy(hpet, (void *)address, sizeof(struct hpet));
					if (!check_sum((struct sdt_header *)hpet))
						warn("Corrupted HPET!");
				} else if (strncmp(header->signature, "APIC", 4) == 0) {
					info("Found MADT");
					memcpy(madt, (void *)address, sizeof(struct madt));
					if (!check_sum((struct sdt_header *)madt))
						warn("Corrupted MADT!");
				}
			}
		}
	} else {
		warn("Wrong RSD signature!");
	}
	kfree(header);
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

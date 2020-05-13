// Important specification: https://uefi.org/sites/default/files/resources/ACPI_6_2.pdf
// HPET: https://www.intel.com/content/dam/www/public/us/en/documents/technical-specifications/software-developers-hpet-spec-1-0a.pdf

#include <acpi/acpi.h>
#include <io/io.h>
#include <lib/lib.h>
#include <lib/stdio.h>
#include <lib/stdlib.h>
#include <memory/alloc.h>
#include <memory/paging.h>
#include <multiboot.h>
#include <stddef.h>
#include <system.h>

struct rsdt *rsdt;
struct fadt *fadt;
struct hpet *hpet;
struct madt *madt;

int check_sum(struct sdt_header *header)
{
	u8 sum = 0;

	for (u32 i = 0; i < header->length; i++)
		sum += ((char *)header)[i];

	return sum == 0;
}

void acpi_init(struct rsdp *rsdp)
{
	// TODO: Fix ACPI table discovering (HPET & MADT missing)
	// TODO: Fix ACPI breaking VESA (why?!)

	struct sdt_header *header = (struct sdt_header *)malloc(sizeof(struct sdt_header));
	rsdt = (struct rsdt *)malloc(sizeof(struct rsdt));
	fadt = (struct fadt *)malloc(sizeof(struct fadt));
	hpet = (struct hpet *)malloc(sizeof(struct hpet));
	madt = (struct madt *)malloc(sizeof(struct madt));

	if (strncmp(rsdp->signature, "RSD PTR ", 8) == 0) {
		memcpy(rsdt, rsdp->rsdt_address, sizeof(struct rsdt) + 32);
		debug("Found RSDT");
		if (!check_sum((struct sdt_header *)rsdt)) {
			warn("Corrupted RSDT!");
		} else {
			u32 *pointer = (u32 *)(rsdt + 1);
			u32 *end = (u32 *)((u8 *)rsdt + rsdt->header.length);

			while (pointer < end) {
				u32 address = *pointer++;
				memcpy(header, (void *)address, sizeof(struct sdt_header));

				if (strncmp(header->signature, "FACP", 4) == 0) {
					debug("Found FADT");
					memcpy(fadt, (void *)address, sizeof(struct fadt));
					if (!check_sum((struct sdt_header *)fadt))
						warn("Corrupted FADT!");
				} else if (strncmp(header->signature, "HPET", 4) == 0) {
					debug("Found HPET");
					memcpy(hpet, (void *)address, sizeof(struct hpet));
					if (!check_sum((struct sdt_header *)hpet))
						warn("Corrupted HPET!");
				} else if (strncmp(header->signature, "APIC", 4) == 0) {
					debug("Found MADT");
					memcpy(madt, (void *)address, sizeof(struct madt));
					if (!check_sum((struct sdt_header *)madt))
						warn("Corrupted MADT!");
				}
			}
		}
	} else {
		warn("Wrong RSD signature!");
	}
	free(header);
}

void acpi_old_init(struct multiboot_tag_old_acpi *tag)
{
	// acpi_init((struct rsdp *)tag->rsdp);
}

void acpi_new_init(struct multiboot_tag_new_acpi *tag)
{
	// acpi_init((struct rsdp *)tag->rsdp);
}

void acpi_poweroff()
{
	// TODO: Add APCI poweroff support
	cli();
	outw(0x604, 0x2000); // QEMU
	outw(0xB004, 0x2000); // Bochs
	outw(0x4004, 0x3400); // VirtualBox
	halt_loop();
}

void reboot()
{
	cli();
	if (fadt->header.revision >= 2 && fadt->flags.reset_support) {
		debug("Reset support!");
		outb(fadt->reset_reg.address, fadt->reset_value);
		halt_loop();
	} else {
		u8 good = 0x02;
		while (good & 0x02)
			good = inb(0x64);
		outb(0x64, 0xFE);
		halt_loop();
	}
}
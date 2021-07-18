// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <string.h>

#include <acpi.h>

/**
 * General SDP
 */

CLEAR static u8 acpi_sdp_verify(struct sdp_header *header)
{
	u8 sum = 0;

	for (u32 i = 0; i < sizeof(struct rsdp); i++)
		sum += (u8)(((u8 *)header)[i]);

	return sum == 0;
}

/**
 * General SDT
 */

CLEAR static u8 acpi_sdt_verify(struct sdt_header *header)
{
	u8 sum = 0;

	for (u32 i = 0; i < header->length; i++)
		sum += (u8)(((u8 *)header)[i]);

	return sum == 0;
}

CLEAR static void *acpi_sdt_find(struct rsdt *rsdt, const char *signature)
{
	u32 entries = (rsdt->header.length - sizeof(rsdt->header)) / 4;

	for (u32 i = 0; i < entries; i++) {
		struct sdt_header *header = (struct sdt_header *)rsdt->sdt_pointer[i];
		if (memcmp(header->signature, signature, 4) == 0) {
			if (acpi_sdt_verify(header))
				return header;
			else
				break;
		}
	}

	return NULL;
}

/**
 * RSDP
 */

CLEAR static struct rsdp *acpi_rsdp_find(void)
{
	uintptr_t ebda = EBDA;
	for (uintptr_t i = ebda; i < 0x100000; i += 16) {
		if (i == ebda + 1024)
			i = BIOS; // Next: BIOS area

		if (memcmp((char *)i, RSDP_MAGIC, 8) == 0)
			return (struct rsdp *)i;
	}

	return NULL;
}

/**
 * General parser
 * TODO: Don't panic?
 */

CLEAR static void acpi_parse(struct rsdp *rsdp)
{
	if (!rsdp || rsdp->header.revision != 0 || !acpi_sdp_verify(&rsdp->header))
		kernel_panic("Invalid RSDP\n");

	struct rsdt *rsdt = rsdp->rsdt;
	if (!rsdt || memcmp(rsdt->header.signature, RSDT_MAGIC, 4) != 0 ||
	    !acpi_sdt_verify(&rsdt->header))
		kernel_panic("Invalid RSDT\n");

	struct madt *madt = acpi_sdt_find(rsdt, MADT_MAGIC);
	struct fadt *fadt = acpi_sdt_find(rsdt, FADT_MAGIC);
	struct hpet *hpet = acpi_sdt_find(rsdt, HPET_MAGIC);

	// TODO!
	UNUSED(madt);
	UNUSED(fadt);
	UNUSED(hpet);
}

CLEAR void acpi_probe(struct boot_information *info)
{
	if (info->acpi.available && info->acpi.rsdp)
		acpi_parse((struct rsdp *)info->acpi.rsdp);
	else
		acpi_parse(acpi_rsdp_find());
}

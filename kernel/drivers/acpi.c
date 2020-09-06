// MIT License, Copyright (c) 2020 Marvin Borner

#include <acpi.h>
#include <assert.h>
#include <cpu.h>
#include <def.h>
#include <mem.h>
#include <print.h>

int check_sdt(struct sdt_header *header)
{
	u8 sum = 0;

	for (u32 i = 0; i < header->length; i++)
		sum += ((char *)header)[i];

	return sum == 0;
}

int check_sdp(struct sdp_header *header)
{
	u8 sum = 0;

	for (u32 i = 0; i < sizeof(struct rsdp); i++)
		sum += ((char *)header)[i];

	return sum == 0;
}

struct rsdp *find_rsdp()
{
	// Main BIOS area
	for (int i = 0xe0000; i < 0xfffff; i++) {
		if (memcmp((u32 *)i, RSDP_MAGIC, 8) == 0)
			return (struct rsdp *)i;
	}

	// Or first KB of EBDA?
	for (int i = 0x100000; i < 0x101000; i++) {
		if (memcmp((u32 *)i, RSDP_MAGIC, 8) == 0)
			return (struct rsdp *)i;
	}

	return NULL;
}

void *find_sdt(struct rsdt *rsdt, const char *signature)
{
	int entries = (rsdt->header.length - sizeof(rsdt->header)) / 4;

	for (int i = 0; i < entries; i++) {
		struct sdt_header *header = (struct sdt_header *)rsdt->sdt_pointer[i];
		if (memcmp(header->signature, signature, 4) == 0) {
			if (check_sdt(header))
				return header;
			else
				break;
		}
	}

	return NULL;
}

void acpi_install()
{
	struct rsdp *rsdp = find_rsdp();
	assert(rsdp && rsdp->header.revision == 0 && check_sdp(&rsdp->header));
	struct rsdt *rsdt = rsdp->rsdt;
	assert(rsdt && memcmp(rsdt->header.signature, RSDT_MAGIC, 4) == 0 &&
	       check_sdt(&rsdt->header));

	madt = find_sdt(rsdt, MADT_MAGIC);
	fadt = find_sdt(rsdt, FADT_MAGIC);
	hpet = find_sdt(rsdt, HPET_MAGIC);
}

void hpet_install()
{
	if (hpet && hpet->legacy_replacement && hpet->comparator_count > 0) {
		struct hpet_registers *r = (struct hpet_registers *)hpet->address.phys;
		printf("HPET tick period: %dns\n", r->features.tick_period / 1000000);
		printf("Periodic support: %d\n", r->timer.periodic_support);
		r->config.enable = 1;
		r->config.legacy_replacement = 1;
		r->timer.enable = 1;
	} else {
		hpet = NULL;
	}
}

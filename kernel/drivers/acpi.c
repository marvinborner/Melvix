// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <def.h>
#include <drivers/acpi.h>
#include <drivers/cpu.h>
#include <mem.h>
#include <print.h>

static int check_sdt(struct sdt_header *header)
{
	u8 sum = 0;

	for (u32 i = 0; i < header->length; i++)
		sum += (u8)(((u8 *)header)[i]);

	return sum == 0;
}

static int check_sdp(struct sdp_header *header)
{
	u8 sum = 0;

	for (u32 i = 0; i < sizeof(struct rsdp); i++)
		sum += (u8)(((u8 *)header)[i]);

	return sum == 0;
}

static struct rsdp *find_rsdp(void)
{
	// Main BIOS area
	for (int i = 0xe0000; i < 0xfffff; i++) {
		if (memcmp((u32 *)i, RSDP_MAGIC, 8) == 0)
			return (struct rsdp *)i;
	}

	// Or first KB of EBDA?
	u8 *ebda = (void *)(*((u16 *)0x40E) << 4);
	for (int i = 0; i < 1024; i += 16) {
		if (memcmp(ebda + i, RSDP_MAGIC, 8) == 0)
			return (struct rsdp *)(ebda + i);
	}

	return NULL;
}

static void *find_sdt(struct rsdt *rsdt, const char *signature)
{
	u32 entries = (rsdt->header.length - sizeof(rsdt->header)) / 4;

	for (u32 i = 0; i < entries; i++) {
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

void acpi_install(void)
{
	struct rsdp *rsdp = find_rsdp();
	assert(rsdp && rsdp->header.revision == 0 && check_sdp(&rsdp->header));
	struct rsdt *rsdt = rsdp->rsdt;
	assert(rsdt && memcmp(rsdt->header.signature, RSDT_MAGIC, 4) == 0 &&
	       check_sdt(&rsdt->header));

	madt = find_sdt(rsdt, MADT_MAGIC);
	fadt = find_sdt(rsdt, FADT_MAGIC);
	hpet = find_sdt(rsdt, HPET_MAGIC);

	madt_install();
}

void hpet_install(u32 period)
{
	if (hpet && hpet->legacy_replacement && hpet->comparator_count > 0) {
		struct hpet_registers *r = (struct hpet_registers *)hpet->address.phys;
		printf("HPET tick period: %dns\n", HPET_MAX_PERIOD / r->tick_period);
		if ((r->timer0 & hpet_periodic_support) == hpet_periodic_support) {
			r->timer0 |= hpet_periodic | hpet_set_accumulator | hpet_enable_timer;
			r->config |= hpet_legacy_replacement;
			r->config |= hpet_enable;
			assert(r->tick_period + period < HPET_MAX_PERIOD);
			r->timer_comparator0 = r->tick_period + period;
			/* r->timer_comparator0 = period; */
		} else {
			hpet = NULL;
		}
	} else {
		hpet = NULL;
	}
}

void madt_install(void)
{
	if (!madt)
		return;

	struct madt_entry_header *entry = &madt->entry;
	while (entry && entry->length) {
		switch (entry->type) {
		case MADT_LOCAL_APIC_ENTRY: {
			struct madt_local_apic_entry *table = (struct madt_local_apic_entry *)entry;
			printf("CPU %b\n", table->flags);
			break;
		}
		case MADT_IO_APIC_ENTRY: {
			/* struct madt_io_apic_entry *table = (struct madt_io_apic_entry *)entry; */
			break;
		}
		case MADT_INT_SRC_OVERRIDE_ENTRY: {
			/* struct madt_int_src_override_entry *table = */
			/* 	(struct madt_int_src_override_entry *)entry; */
			break;
		}
		case MADT_NON_MASKABLE_INT_ENTRY: {
			/* struct madt_non_maskable_int_entry *table = */
			/* 	(struct madt_non_maskable_int_entry *)entry; */
			break;
		}
		case MADT_LOCAL_APIC_OVERRIDE_ENTRY: {
			/* struct madt_local_apic_override_entry *table = */
			/* 	(struct madt_local_apic_override_entry *)entry; */
			break;
		}
		default: {
			break;
		}
		}
		entry = (struct madt_entry_header *)((u32)entry + entry->length);
	}
}

// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>

#include <boot/abstract.h>

struct abstract_memory_map_entry { // Multiboot compatible - kinda e820
	u32 size; // Until next entry
	u32 base; // Low
	u32 base_high;
	u32 length; // Low
	u32 length_high;
	enum {
		BOOT_MEMORY_NONE,
		BOOT_MEMORY_USABLE,
		BOOT_MEMORY_RESERVED,
		BOOT_MEMORY_RECLAIMABLE,
		BOOT_MEMORY_ACPI_NVS,
		BOOT_MEMORY_UNUSABLE,
	} type;
} PACKED;

PROTECTED static struct {
	struct {
		u8 vbe : 1;
		u8 arguments : 1;
		u8 memory_map : 1;
	} has;
	struct {
		void *vbe_mode_info;
		char *arguments;
		struct abstract_memory_map_entry *memory_map;
		u32 memory_map_length;
	} data;
} object = { 0 };

TEMPORARY void abstract_boot_memory_map_set(void *memory_map, u32 memory_map_length)
{
	object.has.memory_map = 1;
	object.data.memory_map = memory_map;
	object.data.memory_map_length = memory_map_length;
}

TEMPORARY void abstract_boot_arguments_set(char *arguments)
{
	object.has.arguments = 1;
	object.data.arguments = arguments;
}

TEMPORARY void abstract_boot_vbe_info_set(void *vbe_mode_info)
{
	object.has.vbe = 1;
	object.data.vbe_mode_info = vbe_mode_info;
}

TEMPORARY static void abstract_boot_memory_map_parse(void)
{
	assert(object.has.memory_map); // Memory map is essential

	struct abstract_memory_map_entry *map = object.data.memory_map;
	u32 length = object.data.memory_map_length;
	u32 count = length / sizeof(*map);

	u32 total = 0;

	/* physical_set_used_all(); */
	for (u32 i = 0; i < count; i++) {
		/* printf("Memory region 0x%x-0x%x\n", mmap->addr, mmap->addr + mmap->len); */
		if (map->type == BOOT_MEMORY_USABLE) {
			total += map->size;
			/* physical_set_free(memory_range_from(map->base, map->length)); */
		} else if (map->type == BOOT_MEMORY_UNUSABLE) {
			printf("Defect memory at 0x%x-0x%x\n", map->base, map->base + map->length);
		} else {
			// Memory is set to 'used' by default
		}

		map++;
	}

	/* physical_total_set(total); */
}

TEMPORARY static void abstract_boot_arguments_parse(void)
{
	if (!object.has.arguments)
		return;

	const char *start = object.data.arguments;
	for (const char *p = start; p && *p; p++) {
		if (*p == ' ')
			start = p + 1;

		/* if (memcmp(start, "nolog", 5) == 0 && !ALPHANUMERIC(start[5])) { */
		/* 	nolog = 1; */
		/* 	start += 5; */
		/* } */
	}
}

TEMPORARY void abstract_boot_finish(void)
{
	abstract_boot_memory_map_parse();
	abstract_boot_arguments_parse();
}

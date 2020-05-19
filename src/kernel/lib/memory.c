#include <lib/stdio.h>
#include <memory/paging.h>
#include <multiboot.h>
#include <stddef.h>
#include <stdint.h>
#include <system.h>

void *memcpy(void *dest, const void *src, u32 count)
{
	const char *sp = (const char *)src;
	char *dp = (char *)dest;
	for (; count != 0; count--)
		*dp++ = *sp++;
	return dest;
}

void *memset(void *dest, char val, u32 count)
{
	char *temp = (char *)dest;
	for (; count != 0; count--)
		*temp++ = val;
	return dest;
}

int memcmp(const void *a_ptr, const void *b_ptr, u32 size)
{
	const u8 *a = (const u8 *)a_ptr;
	const u8 *b = (const u8 *)b_ptr;
	for (u32 i = 0; i < size; i++) {
		if (a[i] < b[i])
			return -1;
		else if (b[i] < a[i])
			return 1;
	}
	return 0;
}

u32 total = 0;
struct multiboot_tag_basic_meminfo *meminfo = NULL;

u32 memory_get_all()
{
	if (total != 0) {
		return total;
	} else if (meminfo != NULL) {
		return meminfo->mem_lower + meminfo->mem_upper;
	} else {
		warn("Got no memory info, guessing size!");
		return 42000; // This should not happen on a modern pc but well idk, 42MB?
	}
}

u32 memory_get_free()
{
	return memory_get_all() /*- paging_get_used_pages() * 4*/;
}

void memory_print()
{
	// TODO: Fix multiboot mem lower/upper
	if (meminfo != NULL) {
		info("Mem lower: 0x%x", meminfo->mem_lower);
		info("Mem upper: 0x%x", meminfo->mem_upper);
	}
	info("Total memory found: %dMiB", (memory_get_all() >> 10) + 1);
	info("Total free memory: %dMiB", (memory_get_free() >> 10) + 1);
}

void memory_info_init(struct multiboot_tag_basic_meminfo *tag)
{
	meminfo = tag;
}

void memory_mmap_init(struct multiboot_tag_mmap *tag)
{
	u32 sum = 0;
	struct multiboot_mmap_entry *mmap;

	for (mmap = ((struct multiboot_tag_mmap *)tag)->entries; (u8 *)mmap < (u8 *)tag + tag->size;
	     mmap = (multiboot_memory_map_t *)((u32)mmap +
					       ((struct multiboot_tag_mmap *)tag)->entry_size)) {
		if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
			debug("Found free memory");
			paging_set_present(mmap->addr, mmap->len >> 12);
			sum += mmap->len;
		} else if (mmap->type == MULTIBOOT_MEMORY_RESERVED) {
			debug("Found reserved memory");
			paging_set_present(mmap->addr, mmap->len >> 12);
			paging_set_used(mmap->addr, mmap->len >> 12);
		} else if (mmap->type == MULTIBOOT_MEMORY_ACPI_RECLAIMABLE) {
			debug("Found ACPI reclaimable memory");
		} else if (mmap->type == MULTIBOOT_MEMORY_NVS) {
			debug("Found NVS memory");
		} else if (mmap->type == MULTIBOOT_MEMORY_BADRAM) {
			warn("Found bad memory!");
		}
	}
	total = sum >> 10; // I want kb
}

int memory_init()
{
	int ret = 0;
	struct multiboot_tag *tag;

	for (tag = (struct multiboot_tag *)(multiboot_address + 8);
	     tag->type != MULTIBOOT_TAG_TYPE_END;
	     tag = (struct multiboot_tag *)((u8 *)tag + ((tag->size + 7) & ~7))) {
		if (tag->type == MULTIBOOT_TAG_TYPE_BASIC_MEMINFO) {
			info("Got memory info");
			memory_info_init((struct multiboot_tag_basic_meminfo *)tag);
		} else if (tag->type == MULTIBOOT_TAG_TYPE_MMAP) {
			info("Got memory map");
			memory_mmap_init((struct multiboot_tag_mmap *)tag);
			ret = 1;
		}
	}
	return ret;
}
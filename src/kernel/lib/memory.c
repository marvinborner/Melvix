#include <stddef.h>
#include <stdint.h>
#include <kernel/system.h>
#include <kernel/lib/stdio.h>
#include <kernel/memory/paging.h>
#include <kernel/multiboot.h>

void *memcpy(void *dest, const void *src, size_t count)
{
	const char *sp = (const char *)src;
	char *dp = (char *)dest;
	for (; count != 0; count--)
		*dp++ = *sp++;
	return dest;
}

void *memset(void *dest, char val, size_t count)
{
	char *temp = (char *)dest;
	for (; count != 0; count--)
		*temp++ = val;
	return dest;
}

int memcmp(const void *a_ptr, const void *b_ptr, size_t size)
{
	const unsigned char *a = (const unsigned char *)a_ptr;
	const unsigned char *b = (const unsigned char *)b_ptr;
	for (size_t i = 0; i < size; i++) {
		if (a[i] < b[i])
			return -1;
		else if (b[i] < a[i])
			return 1;
	}
	return 0;
}

uint32_t total = 0;
struct multiboot_tag_basic_meminfo *meminfo = NULL;

uint32_t memory_get_all()
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

uint32_t memory_get_free()
{
	return memory_get_all() - paging_get_used_pages() * 4;
}

void memory_print()
{
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
	uint32_t sum = 0;
	struct multiboot_mmap_entry *mmap;

	for (mmap = ((struct multiboot_tag_mmap *)tag)->entries;
	     (multiboot_uint8_t *)mmap < (multiboot_uint8_t *)tag + tag->size;
	     mmap = (multiboot_memory_map_t *)((uint32_t)mmap +
					       ((struct multiboot_tag_mmap *)tag)->entry_size)) {
		if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
			log("Found free memory");
			paging_set_present(mmap->addr, mmap->len >> 12);
			sum += mmap->len;
		} else if (mmap->type == MULTIBOOT_MEMORY_RESERVED) {
			log("Found reserved memory");
			paging_set_present(mmap->addr, mmap->len >> 12);
			paging_set_used(mmap->addr, mmap->len >> 12);
		} else if (mmap->type == MULTIBOOT_MEMORY_ACPI_RECLAIMABLE) {
			log("Found ACPI reclaimable memory");
		} else if (mmap->type == MULTIBOOT_MEMORY_NVS) {
			log("Found NVS memory");
		} else if (mmap->type == MULTIBOOT_MEMORY_BADRAM) {
			warn("Found bad memory!");
		}
	}
	total = sum >> 10; // I want kb
}

int memory_init(uint32_t multiboot_address)
{
	int ret = 0;
	struct multiboot_tag *tag;

	for (tag = (struct multiboot_tag *)(multiboot_address + 8);
	     tag->type != MULTIBOOT_TAG_TYPE_END;
	     tag = (struct multiboot_tag *)((multiboot_uint8_t *)tag + ((tag->size + 7) & ~7))) {
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

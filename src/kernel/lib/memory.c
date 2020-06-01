#include <lib/stdio.h>
#include <memory/mmap.h>
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

// TODO: Move memory lib!
u32 total = 0;
struct multiboot_tag_basic_meminfo *meminfo = NULL;
struct multiboot_tag_mmap *mmap = NULL;

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
	int free_memory = memory_get_all() - 42 * 4; // TODO: Fix free memory
	if (free_memory < 0)
		return 0;
	else
		return free_memory;
}

void memory_print()
{
	// TODO: Fix multiboot mem lower/upper
	if (meminfo != NULL) {
		info("Mem lower: 0x%x", meminfo->mem_lower);
		info("Mem upper: 0x%x", meminfo->mem_upper);
	}
	info("Total memory found: %dMiB", (memory_get_all() >> 10));
	info("Total used memory: %dMiB", ((memory_get_all() - memory_get_free()) >> 10));
	info("Total free memory: %dMiB", (memory_get_free() >> 10));
}

void memory_info_init()
{
}

void memory_mmap_init(struct multiboot_tag_mmap *tag)
{
	u32 sum = 0;
	struct multiboot_mmap_entry *mmap;

	for (mmap = ((struct multiboot_tag_mmap *)tag)->entries; (u8 *)mmap < (u8 *)tag + tag->size;
	     mmap = (multiboot_memory_map_t *)((u32)mmap +
					       ((struct multiboot_tag_mmap *)tag)->entry_size)) {
		debug("Found memory of type %d from 0x%x-0x%x: %dKiB", mmap->type, (u32)mmap->addr,
		      (u32)mmap->addr + (u32)mmap->len, mmap->len >> 10);
		sum += mmap->len;

		// Translate to pages
		if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
			for (u32 i = 0; i < mmap->len; i += 0x1000) {
				if (mmap->addr + i > 0xFFFFFFFF)
					break;
				mmap_address_set_free((mmap->addr + i) & 0xFFFFF000);
			}
		} else {
			for (u32 i = 0; i < mmap->len; i += 0x1000) {
				if (mmap->addr + i > 0xFFFFFFFF)
					break;
				mmap_address_set_used((mmap->addr + i) & 0xFFFFF000);
			}
		}

		total = sum >> 10; // I want kb
	}
}

void memory_init()
{
	struct multiboot_tag *tag = NULL;

	for (tag = (struct multiboot_tag *)(multiboot_address + 8);
	     tag->type != MULTIBOOT_TAG_TYPE_END;
	     tag = (struct multiboot_tag *)((u8 *)tag + ((tag->size + 7) & ~7))) {
		if (tag->type == MULTIBOOT_TAG_TYPE_BASIC_MEMINFO) {
			info("Got memory info");
			meminfo = (struct multiboot_tag_basic_meminfo *)tag;
		} else if (tag->type == MULTIBOOT_TAG_TYPE_MMAP) {
			info("Got memory map");
			mmap = (struct multiboot_tag_mmap *)tag;
		}
	}

	assert(mmap && meminfo);
	mmap_init(meminfo->mem_lower + meminfo->mem_upper);
	memory_mmap_init(mmap);
	mmap_init_finalize();
}

void bss_clean()
{
	u32 start = &bss_start;
	u32 end = &kernel_end;
	memset(start, 0, end - start);
}
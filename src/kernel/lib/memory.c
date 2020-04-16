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

struct multiboot_tag_basic_meminfo *meminfo = NULL;

uint32_t memory_get_all()
{
	if (meminfo != NULL) {
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
	info("Mem lower: 0x%x", meminfo->mem_lower);
	info("Mem upper: 0x%x", meminfo->mem_upper);
	info("Total memory found: %dMiB", (memory_get_all() >> 10) + 1);
	info("Total free memory: %dMiB", (memory_get_free() >> 10) + 1);
}

void memory_init(struct multiboot_tag_basic_meminfo *tag)
{
	meminfo = tag;
}

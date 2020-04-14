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

multiboot_info_t *multiboot_header;

void memory_print()
{
	if (multiboot_header->flags & MULTIBOOT_INFO_MEMORY) {
		serial_printf("Mem lower: 0x%x", multiboot_header->mem_lower);
		serial_printf("Mem upper: 0x%x", multiboot_header->mem_upper);
	} else {
		serial_printf("No memory information available!");
	}
}

void memory_init(multiboot_info_t *grub_header)
{
	multiboot_header = grub_header;
}

uint32_t memory_get_free()
{
	return multiboot_header->mem_upper - paging_get_used_pages() * 4;
}

uint32_t memory_get_all()
{
	return multiboot_header->mem_upper;
}
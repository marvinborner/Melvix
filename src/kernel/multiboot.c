#include <stdint.h>
#include <kernel/system.h>
#include <kernel/multiboot.h>
#include <kernel/lib/lib.h>
#include <kernel/lib/stdio.h>

void multiboot_parse(uint32_t multiboot_address)
{
	struct multiboot_tag *tag;

	for (tag = (struct multiboot_tag *)(multiboot_address + 8);
	     tag->type != MULTIBOOT_TAG_TYPE_END;
	     tag = (struct multiboot_tag *)((multiboot_uint8_t *)tag + ((tag->size + 7) & ~7))) {
		switch (tag->type) {
		case MULTIBOOT_TAG_TYPE_CMDLINE:
			info("Got cmdline");
			break;
		case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
			info("Got bootloader name: %s",
			     ((struct multiboot_tag_string *)tag)->string);
			break;
		case MULTIBOOT_TAG_TYPE_MODULE:
			info("Got modules");
			break;
		case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
			info("Got memory info");
			memory_info_init((struct multiboot_tag_basic_meminfo *)tag);
			break;
		case MULTIBOOT_TAG_TYPE_BOOTDEV:
			info("Got boot device");
			break;
		case MULTIBOOT_TAG_TYPE_MMAP:
			info("Got memory map");
			memory_mmap_init((struct multiboot_tag_mmap *)tag);
			break;
		case MULTIBOOT_TAG_TYPE_VBE:
			info("Got VBE info");
			break;
		case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
			info("Got framebuffer info");
			break;
		case MULTIBOOT_TAG_TYPE_APM:
			info("Got APM table");
			break;
		case MULTIBOOT_TAG_TYPE_EFI32:
			info("Got EFI32");
			break;
		case MULTIBOOT_TAG_TYPE_ACPI_OLD:
			info("Got ACPI table");
			break;
		case MULTIBOOT_TAG_TYPE_ACPI_NEW:
			info("Got new ACPI table");
			break;
		case MULTIBOOT_TAG_TYPE_NETWORK:
			info("Got network info");
			break;
		case MULTIBOOT_TAG_TYPE_EFI_MMAP:
			info("Got EFI memory map");
			break;
		case MULTIBOOT_TAG_TYPE_EFI_BS:
			info("Got EFI boot services");
			break;
		case MULTIBOOT_TAG_TYPE_EFI32_IH:
			info("Got EFI image handler pointer");
			break;
		case MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR:
			info("Got image load base address");
			break;
		}
	}
}

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
		//log("Tag 0x%x, Size 0x%x", tag->type, tag->size);
		switch (tag->type) {
		case MULTIBOOT_TAG_TYPE_CMDLINE:
			//info("Command line: %s", ((struct multiboot_tag_string *)tag)->string);
			break;
		case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
			//info("Bootloader name: %s", ((struct multiboot_tag_string *)tag)->string);
			break;
		case MULTIBOOT_TAG_TYPE_MODULE:
			break;
		case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
			log("YAY");
			memory_init((struct multiboot_tag_basic_meminfo *)tag);
			break;
		case MULTIBOOT_TAG_TYPE_BOOTDEV:
			break;
		case MULTIBOOT_TAG_TYPE_MMAP:
			break;
		case MULTIBOOT_TAG_TYPE_VBE:
			break;
		case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
			break;
		case MULTIBOOT_TAG_TYPE_APM:
			break;
		case MULTIBOOT_TAG_TYPE_EFI32:
			break;
		case MULTIBOOT_TAG_TYPE_ACPI_OLD:
			break;
		case MULTIBOOT_TAG_TYPE_ACPI_NEW:
			break;
		case MULTIBOOT_TAG_TYPE_NETWORK:
			break;
		case MULTIBOOT_TAG_TYPE_EFI_MMAP:
			break;
		case MULTIBOOT_TAG_TYPE_EFI_BS:
			break;
		case MULTIBOOT_TAG_TYPE_EFI32_IH:
			break;
		case MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR:
			break;
		}
	}
}

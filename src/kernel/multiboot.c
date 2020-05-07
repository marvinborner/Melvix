#include <acpi/acpi.h>
#include <graphics/vesa.h>
#include <lib/lib.h>
#include <lib/stdio.h>
#include <multiboot.h>
#include <smbios/smbios.h>
#include <stdint.h>
#include <system.h>

void multiboot_parse(u32 multiboot_address)
{
	struct multiboot_tag *tag;

	for (tag = (struct multiboot_tag *)(multiboot_address + 8);
	     tag->type != MULTIBOOT_TAG_TYPE_END;
	     tag = (struct multiboot_tag *)((multiboot_u8 *)tag + ((tag->size + 7) & ~7))) {
		switch (tag->type) {
		case MULTIBOOT_TAG_TYPE_CMDLINE:
			// TODO: Add cmdline config support
			debug("Got cmdline: %s", ((struct multiboot_tag_string *)tag)->string);
			break;
		case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
			debug("Got bootloader name: %s",
			      ((struct multiboot_tag_string *)tag)->string);
			break;
		case MULTIBOOT_TAG_TYPE_MODULE:
			debug("Got modules");
			break;
		case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
			debug("Got memory info");
			memory_info_init((struct multiboot_tag_basic_meminfo *)tag);
			break;
		case MULTIBOOT_TAG_TYPE_BOOTDEV:
			debug("Got boot device");
			break;
		case MULTIBOOT_TAG_TYPE_MMAP:
			debug("Got memory map");
			memory_mmap_init((struct multiboot_tag_mmap *)tag);
			break;
		case MULTIBOOT_TAG_TYPE_VBE:
			debug("Got VBE");
			break;
		case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
			debug("Got framebuffer");
			break;
		case MULTIBOOT_TAG_TYPE_APM:
			debug("Got APM table");
			break;
		case MULTIBOOT_TAG_TYPE_EFI32:
			debug("Got EFI32");
			break;
		case MULTIBOOT_TAG_TYPE_SMBIOS:
			// GRUB doesn't detect SMBIOS on QEMU!
			debug("Got SMBIOS table");
			smbios_init((struct multiboot_tag_smbios *)tag);
			break;
		case MULTIBOOT_TAG_TYPE_ACPI_OLD:
			debug("Got ACPI 1.0 table");
			acpi_old_init((struct multiboot_tag_old_acpi *)tag);
			break;
		case MULTIBOOT_TAG_TYPE_ACPI_NEW:
			debug("Got ACPI 2.0 table");
			acpi_new_init((struct multiboot_tag_new_acpi *)tag);
			break;
		case MULTIBOOT_TAG_TYPE_NETWORK:
			debug("Got network debug");
			break;
		case MULTIBOOT_TAG_TYPE_EFI_MMAP:
			debug("Got EFI memory map");
			break;
		case MULTIBOOT_TAG_TYPE_EFI_BS:
			debug("Got EFI boot services");
			break;
		case MULTIBOOT_TAG_TYPE_EFI32_IH:
			debug("Got EFI image handler pointer");
			break;
		case MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR:
			debug("Got image load base address");
			break;
		}
	}
}
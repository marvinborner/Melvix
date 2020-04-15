#include <kernel/fs/load.h>
#include <kernel/system.h>
#include <kernel/memory/alloc.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/lib.h>

void load_binaries()
{
	userspace = (uint32_t)kmalloc(10000);
	font = (struct font *)kmalloc(100000); // High quality shit

	if (multiboot_header->boot_device != 0xE0FFFFFF) {
		panic("Unsupported boot drive!");
	} else {
		/*
		char *font_p[] = { "FONT.BIN" };
		struct iso9660_entity *font_e = ISO9660_get(font_p, 1);
		if (!font_e)
			panic("Font not found!");
		ATAPI_granular_read(1 + (font_e->length / 2048), font_e->lba, (uint8_t *)font);
		kfree(font_e);

		char *user_p[] = { "USER.BIN" };
		struct iso9660_entity *user_e = ISO9660_get(user_p, 1);
		if (!user_e)
			panic("Userspace binary not found!");
		ATAPI_granular_read(1 + (user_e->length / 2048), user_e->lba, (uint8_t *)userspace);
		kfree(user_e);

		if (font->magic != 0xf0f0f0f0) {
			serial_printf("0x%x: WRONG FONT MAGIC!", font->magic);
			halt_loop();
		}
		*/
	}
	vga_log("Successfully loaded binaries");
}

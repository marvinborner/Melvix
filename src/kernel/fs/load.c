#include <kernel/fs/load.h>
#include <kernel/fs/marfs/marfs.h>
#include <kernel/fs/ata_pio.h>
#include <kernel/fs/atapi_pio.h>
#include <kernel/system.h>
#include <kernel/fs/iso9660/iso9660.h>
#include <kernel/memory/alloc.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/lib.h>

void load_binaries()
{
	userspace = (uint32_t)kmalloc(10000);
	font = (struct font *)kmalloc(100000);
	; // High quality shit

	uint8_t boot_drive_id = (uint8_t)(*((uint8_t *)0x9000));
	if (boot_drive_id != 0xE0) {
		struct ata_interface *primary_master = new_ata(1, 0x1F0);
		marfs_init(primary_master);
		marfs_read_whole_file(4, (uint8_t *)userspace);
		marfs_read_whole_file(5, (uint8_t *)font);
	} else {
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
	}
	vga_log("Successfully loaded binaries");
}

#define MAX_LOADERS 16

loader_t **loaders = 0;
uint8_t last_loader = 0;

uint32_t last_load_loc = 0x400000;

void loader_init()
{
	serial_printf("Setting up loader");
	loaders = (loader_t **)kmalloc(MAX_LOADERS * sizeof(uint32_t));
	memset(loaders, 0, MAX_LOADERS * sizeof(uint32_t));
}

void register_loader(loader_t *load)
{
	if (last_loader + 1 > MAX_LOADERS)
		return;
	if (!load)
		return;
	loaders[last_loader] = load;
	last_loader++;
	serial_printf("Registered %s loader as id %d", load->name, last_loader - 1);
}

uint32_t loader_get_unused_load_location()
{
	last_load_loc += 0x400000;
	return last_load_loc;
}

uint8_t exec_start(uint8_t *buf)
{
	for (int i = 0; i < MAX_LOADERS; i++) {
		if (!loaders[i])
			break;
		void *priv = loaders[i]->probe(buf);
		if (priv) {
			return loaders[i]->start(buf, priv);
		}
	}
	return 0;
}

#include <stddef.h>
#include <kernel/multiboot.h>
#include <kernel/system.h>
#include <kernel/smbios/smbios.h>

void smbios_init(struct multiboot_tag_smbios *tag) {
	log("%s", ((struct smbios_0 *)tag->tables)->vendor);
	return;
}

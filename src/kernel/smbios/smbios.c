#include <multiboot.h>
#include <smbios/smbios.h>
#include <stddef.h>
#include <system.h>

void smbios_init(struct multiboot_tag_smbios *tag)
{
	log("%s", ((struct smbios_0 *)tag->tables)->vendor);
	return;
}
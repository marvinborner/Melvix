#include <stddef.h>
#include <multiboot.h>
#include <system.h>
#include <smbios/smbios.h>

void smbios_init(struct multiboot_tag_smbios *tag)
{
	log("%s", ((struct smbios_0 *)tag->tables)->vendor);
	return;
}
#include <kernel/graphics/vesa.h>
#include <stddef.h>
#include <kernel/smbios/smbios.h>

struct smbios_entry *smbios = 0;

size_t smbios_table_len(struct smbios_header *header)
{
	size_t i;
	const char *strtab = (char *)header + header->length;
	for (i = 1; strtab[i - 1] != '\0' || strtab[i] != '\0'; i++)
		;
	return header->length + i + 1;
}

struct smbios_entry *get_smbios()
{
	if (smbios != 0)
		return smbios;

	char *mem = (char *)0xF0000;
	int length, i;
	unsigned char checksum;
	while ((unsigned int)mem < 0x100000) {
		if (mem[0] == '_' && mem[1] == 'S' && mem[2] == 'M' && mem[3] == '_') {
			length = mem[5];
			checksum = 0;
			for (i = 0; i < length; i++) {
				checksum += mem[i];
			}
			if (checksum == 0)
				break;
		}
		mem += 16;
	}

	if ((unsigned int)mem == 0x100000) {
		warn("No SMBIOS found!");
		return 0;
	}

	smbios = (struct smbios_entry *)mem;
	if (smbios->major_version != 2)
		warn("Non-supported SMBIOS version");
	smbios_table((struct smbios_header *)mem);
	return smbios;
}

void smbios_table(struct smbios_header *header)
{
	// struct smbios_0 *table = (struct smbios_0 *) (header + sizeof(struct smbios_header));
	// log("\n\n %d", table->bios_version);
}
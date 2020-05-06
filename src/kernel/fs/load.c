#include <fs/load.h>
#include <system.h>
#include <lib/stdio.h>
#include <lib/lib.h>
#include <fs/ext2.h>

void load_binaries()
{
	font = (struct font *)read_file("/bin/font");

	log("Successfully loaded binaries");
}
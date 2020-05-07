#include <fs/ext2.h>
#include <fs/load.h>
#include <lib/lib.h>
#include <lib/stdio.h>
#include <system.h>

void load_binaries()
{
	font = (struct font *)read_file("/bin/font");

	log("Successfully loaded binaries");
}
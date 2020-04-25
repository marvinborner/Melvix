#include <kernel/fs/load.h>
#include <kernel/system.h>
#include <kernel/memory/alloc.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/lib.h>

void load_binaries()
{
	//userspace = (uint32_t)read_file("/bin/user.bin");
	font = (struct font *)read_file("/bin/font.bin");

	log("Successfully loaded binaries");
}

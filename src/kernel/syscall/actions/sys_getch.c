#include <stdint.h>
#include <kernel/lib/stdio.h>
#include <kernel/io/io.h>

uint32_t sys_getch()
{
	sti();
	uint32_t key = getch();
	cli();
	return key;
}

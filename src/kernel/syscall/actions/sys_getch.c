#include <stdint.h>
#include <kernel/lib/stdio.h>
#include <kernel/input/input.h>
#include <kernel/lib/lib.h>
#include <kernel/lib/string.h>

uint32_t sys_getch()
{
	return (uint32_t)getch();
}
#include <stdint.h>
#include <kernel/lib/stdio.h>
#include <kernel/lib/string.h>

uint32_t sys_putch(char ch)
{
	putch(ch);
	return 0;
}
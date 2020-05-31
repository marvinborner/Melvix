#include <lib/lib.h>
#include <lib/stdio.h>
#include <lib/stdlib.h>
#include <stdint.h>

// TODO: Use global formatting function
// TODO: Fix fixed buffer size
void vprintf(const char *fmt, va_list args)
{
	char buf[1024];
	memset(buf, 0, 1024);
	vsprintf(buf, fmt, args);

	for (u32 i = 0; i < strlen(buf); i++)
		putch(buf[i]);
}
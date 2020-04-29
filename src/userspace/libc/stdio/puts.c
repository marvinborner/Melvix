#include <stdint.h>
#include <stdio.h>
#include <string.h>

void puts(char *data)
{
	for (u32 i = 0; i < strlen(data); i++)
		putch(data[i]);
}
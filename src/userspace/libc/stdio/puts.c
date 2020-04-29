#include <stdint.h>
#include <stdio.h>
#include <string.h>

void puts(char *data)
{
	for (u8 i = 0; i < strlen(data); i++)
		putch(data[i]);
}
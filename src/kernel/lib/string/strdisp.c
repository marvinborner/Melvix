#include <kernel/lib/string.h>

void strdisponce(char *str)
{
	for (size_t i = sizeof(str) + 2; i > 0; i--)
		str[i] = str[i - 1];
	str[0] = 0;
}

void strdisp(char *str, int n)
{
	for (int i = 0; i < n; i++)
		strdisponce(str);
}
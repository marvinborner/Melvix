// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>

u32 strlen(const char *str)
{
	u32 len = 0;
	while (str[len])
		len++;
	return len;
}

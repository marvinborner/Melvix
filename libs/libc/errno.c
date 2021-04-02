// MIT License, Copyright (c) 2021 Marvin Borner

#include <def.h>
#include <errno.h>
#ifdef USER

static u32 error = 0;

u32 *__errno(void)
{
	return &error;
}

#endif

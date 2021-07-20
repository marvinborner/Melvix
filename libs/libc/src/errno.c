// MIT License, Copyright (c) 2021 Marvin Borner

#include <errno.h>

static int error = 0;

int *__errno(void)
{
	return &error;
}

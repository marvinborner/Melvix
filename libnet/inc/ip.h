// MIT License, Copyright (c) 2020 Marvin Borner
// Most net/ip handlers are in the kernel space
// This is a userspace wrapper for some things

#ifndef IP_H
#define IP_H

#include <def.h>

int ip_pton(const char *src, u32 *dst);

#endif

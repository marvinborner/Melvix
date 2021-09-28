// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef MANAGEMENT_MEMORY_RANGE_H
#define MANAGEMENT_MEMORY_RANGE_H

#include <def.h>

struct memory_range {
	u32 base;
	u32 size;
};

#define memory_range(base, size) ((struct memory_range){ (u32)(base), (size) })

u8 memory_readable_range(struct memory_range vrange);
u8 memory_writable_range(struct memory_range vrange);
#define memory_readable(addr) memory_readable_range(memory_range((addr), 1))
#define memory_writable(addr) memory_writable_range(memory_range((addr), 1))

#endif

/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#ifndef CORE_REGISTERS_H
#define CORE_REGISTERS_H

#include <def.h>

#define REG_GET(reg)                                                                               \
	__extension__({                                                                            \
		u32 out;                                                                           \
		__asm__ volatile("movl %%" reg ", %%eax" : "=a"(out));                             \
		out;                                                                               \
	})
#define REG_SET(reg, val) (__asm__ volatile("movl %%eax, %%" reg ::"a"(val)))

#endif

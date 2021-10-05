/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#ifndef CORE_IO_H
#define CORE_IO_H

#include <def.h>

#define IO_IN(size, letter, port)                                                                  \
	__extension__({                                                                            \
		u##size out;                                                                       \
		__asm__ volatile("in" letter " %1, %0" : "=a"(out) : "Nd"(port));                  \
		out;                                                                               \
	})
#define IO_INB(port) IO_IN(8, "b", (u16)(port))
#define IO_INW(port) IO_IN(16, "w", (u16)(port))
#define IO_INL(port) IO_IN(32, "l", (u16)(port))

#define IO_OUT(letter, port, data) __asm__ volatile("out" letter " %0, %1" ::"a"(data), "Nd"(port))
#define IO_OUTB(port, data) IO_OUT("b", (u16)(port), (u8)(data))
#define IO_OUTW(port, data) IO_OUT("w", (u16)(port), (u16)(data))
#define IO_OUTL(port, data) IO_OUT("l", (u16)(port), (u32)(data))

#endif

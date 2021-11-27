/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#ifndef MANAGEMENT_INTERRUPT_HANDLE_H
#define MANAGEMENT_INTERRUPT_HANDLE_H

#include <def.h>

struct interrupt_frame {
	u32 gs, fs, es, ds;
	u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
	u32 interrupt, error;
	u32 eip, cs, eflags;
} PACKED;

void *interrupt_handler(void *data);

#endif

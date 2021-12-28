/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#ifndef EVENTS_INTERRUPT_H
#define EVENTS_INTERRUPT_H

#include <def.h>

struct interrupt_data {
	u32 gs, fs, es, ds;
	u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
	u32 interrupt, error;
	u32 eip, cs, eflags;
} PACKED;

struct interrupt_data *interrupt_handler(struct interrupt_data *frame);

#endif

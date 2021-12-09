/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <def.h>

#include <core/descriptors/interrupt.h>

#define IDT_TYPE_TASK 0x5
#define IDT_TYPE_INTERRUPT 0xe
#define IDT_TYPE_TRAP 0xf
#define IDT_PRIV(priv) (priv << 5)
#define IDT_PRESENT (1 << 7)

struct idtr {
	u16 limit;
	u32 ptr;
	u32 pad;
} PACKED;

struct idt_descriptor {
	u16 base_low;
	u16 selector; // cs
	u8 zero;
	u8 flags;
	u16 base_high;
} PACKED;

PROTECTED static struct idt_descriptor idt_descriptors[256] = { 0 };

PROTECTED static struct idtr idtr = {
	sizeof(idt_descriptors) - 1,
	(u32)idt_descriptors,
	0,
};

PROTECTED extern u32 interrupt_table[];

TEMPORARY void idt_init(void)
{
	for (u16 i = 0; i < COUNT(idt_descriptors); i++) {
		u32 base = (u32)interrupt_table[i];
		u8 type = i > 32 ? IDT_TYPE_INTERRUPT : IDT_TYPE_TRAP;
		idt_descriptors[i] = (struct idt_descriptor){
			.base_low = base & 0xffff,
			.selector = 0x08,
			.zero = 0,
			.flags = type | IDT_PRIV(0) | IDT_PRESENT,
			.base_high = base >> 16,
		};
	}

	__asm__ volatile("lidt %0" : : "m"(idtr));
}

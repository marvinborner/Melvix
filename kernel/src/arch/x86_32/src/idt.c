// MIT License, Copyright (c) 2021 Marvin Borner

#include <gdt.h>
#include <idt.h>
#include <kernel.h>

#define IDT_TYPE_TASK 0x5
#define IDT_TYPE_INTERRUPT 0xe
#define IDT_TYPE_TRAP 0xf

struct idtr {
	u16 limit;
	u32 ptr;
	u32 pad;
} PACKED;

struct idt_descriptor {
	u16 base_low;
	u16 selector; // cs
	u8 zero;
	struct {
		u8 type : 4;
		u8 storage : 1; // 0 for int/trap
		u8 privilege : 2;
		u8 present : 1;
	} flags;
	u16 base_high;
} PACKED;

PROTECTED static struct idt_descriptor idt_descriptors[256] = { 0 };

PROTECTED static struct idtr idtr = {
	sizeof(idt_descriptors) - 1,
	(uintptr_t)idt_descriptors,
	0,
};

struct interrupt_frame {
	u32 gs, fs, es, ds;
	u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
	u32 int_no, err_code;
	u32 eip, cs, eflags;
} PACKED;

ATTR((interrupt)) static void handler(struct interrupt_frame *frame)
{
	UNUSED(frame);
}

CLEAR void idt_init(void)
{
	for (u16 i = 0; i < COUNT(idt_descriptors); i++) {
		u32 base = (u32)&handler;
		u8 type = i > 32 ? IDT_TYPE_INTERRUPT : IDT_TYPE_TRAP;
		idt_descriptors[i] = (struct idt_descriptor){
			.base_low = base & 0xffff,
			.selector = GDT_CODE_SEGMENT,
			.zero = 0,
			.flags.type = type,
			.flags.storage = 0,
			.flags.privilege = 0,
			.flags.present = 1,
			.base_high = base >> 16,
		};
	}

	__asm__ volatile("lidt %0" : : "m"(idtr));
	__asm__ volatile("sti");
}

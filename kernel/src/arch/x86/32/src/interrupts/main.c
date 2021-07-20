// MIT License, Copyright (c) 2021 Marvin Borner

#include <interrupts/main.h>
#include <interrupts/pic.h>
#include <kernel.h>

struct interrupt_frame {
	u32 gs, fs, es, ds;
	u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
	u32 int_no, err_code;
	u32 eip, cs, eflags;
} PACKED;

u32 interrupt_handler(u32 esp);
u32 interrupt_handler(u32 esp)
{
	struct interrupt_frame *frame = (struct interrupt_frame *)esp;
	pic_ack(frame->int_no);
	return esp;
}

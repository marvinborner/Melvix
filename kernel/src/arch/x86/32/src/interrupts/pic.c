// MIT License, Copyright (c) 2021 Marvin Borner

#include <cpu.h>
#include <interrupts/pic.h>

#define PIC1 0x20
#define PIC1_COMMAND PIC1
#define PIC1_OFFSET 0x20
#define PIC1_DATA (PIC1 + 1)

#define PIC2 0xa0
#define PIC2_COMMAND PIC2
#define PIC2_OFFSET 0x28
#define PIC2_DATA (PIC2 + 1)

#define ICW1_ICW4 0x01
#define ICW1_INIT 0x10

CLEAR static void pic_wait(void)
{
	__asm__ volatile("jmp 1f\n\t"
			 "1:\n\t"
			 "    jmp 2f\n\t"
			 "2:");
}

CLEAR void pic_init(void)
{
	// Initialize
	outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
	pic_wait();
	outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
	pic_wait();

	// Remap
	outb(PIC1_DATA, PIC1_OFFSET);
	pic_wait();
	outb(PIC2_DATA, PIC2_OFFSET);
	pic_wait();

	outb(PIC1_DATA, 0x04);
	pic_wait();
	outb(PIC2_DATA, 0x02);
	pic_wait();

	// Set 8086 mode
	outb(PIC1_DATA, 0x01);
	pic_wait();
	outb(PIC2_DATA, 0x01);
	pic_wait();

	outb(PIC1_DATA, 0x00);
	pic_wait();
	outb(PIC2_DATA, 0x00);
	pic_wait();
}

void pic_ack(u32 int_no)
{
	if (int_no >= 40)
		outb(PIC2, 0x20);

	outb(PIC1, 0x20);
}

/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */
// 8259 is the Programmable Interrupt Controller (PIC)

#include <err.h>

#include <core/io.h>
#include <ports/8259.h>

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

#define WAIT()                                                                                     \
	__asm__ volatile("jmp 1f\n\t"                                                              \
			 "1:\n\t"                                                                  \
			 "    jmp 2f\n\t"                                                          \
			 "2:")

static void ack(u32 interrupt)
{
	if (interrupt >= 40)
		IO_OUTB(PIC2, 0x20);
	IO_OUTB(PIC1, 0x20);
}

static err request(u32 request, va_list ap)
{
	switch (request) {
	case PORT_8259_ACK:
		ack(va_arg(ap, u32));
		return ERR_OK;
	default:
		return -ERR_INVALID_ARGUMENTS;
	}
}

static err probe(void)
{
	// TODO: Probe?
	return ERR_OK;
}

static err setup(void)
{
	// Initialize
	IO_OUTB(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
	WAIT();
	IO_OUTB(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
	WAIT();

	// Remap
	IO_OUTB(PIC1_DATA, PIC1_OFFSET);
	WAIT();
	IO_OUTB(PIC2_DATA, PIC2_OFFSET);
	WAIT();

	IO_OUTB(PIC1_DATA, 0x04);
	WAIT();
	IO_OUTB(PIC2_DATA, 0x02);
	WAIT();

	// Set 8086 mode
	IO_OUTB(PIC1_DATA, 0x01);
	WAIT();
	IO_OUTB(PIC2_DATA, 0x01);
	WAIT();

	IO_OUTB(PIC1_DATA, 0x00);
	WAIT();
	IO_OUTB(PIC2_DATA, 0x00);
	WAIT();

	return ERR_OK;
}

PROTECTED struct port port_8259 = {
	.type = PORT_8259,
	.request = request,
	.probe = probe,
	.setup = setup,
};

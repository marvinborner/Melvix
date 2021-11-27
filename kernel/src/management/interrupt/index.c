/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#include <print.h>

#include <management/interrupt/index.h>

PROTECTED static interrupt_t interrupts[0xff] = { 0 };

TEMPORARY void interrupt_add(u32 interrupt, interrupt_t handler)
{
	if (interrupts[interrupt])
		log("Overwriting interrupt handler %x", interrupt);

	interrupts[interrupt] = handler;
}

interrupt_t interrupt_get(u32 interrupt)
{
	if (interrupt >= COUNT(interrupts))
		return NULL;
	return interrupts[interrupt];
}

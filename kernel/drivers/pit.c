// MIT License, Copyright (c) 2021 Marvin Borner

#include <def.h>
#include <drivers/cpu.h>
#include <drivers/pit.h>
#include <timer.h>

CLEAR static void pit_phase(u32 hz)
{
	u32 divisor = 3579545 / 3 / hz;
	outb(0x43, 0x36); // 01 10 11 0b // CTR, RW, MODE, BCD
	outb(0x40, (u8)(divisor & 0xFF));
	outb(0x40, (u8)(divisor >> 8));
}

CLEAR void pit_install(void)
{
	pit_phase(1000);
	timer_install_handler();
}

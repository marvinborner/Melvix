// MIT License, Copyright (c) 2020 Marvin Borner

#include <cpu.h>
#include <def.h>
#include <interrupts.h>

static u32 timer_ticks = 0;

void timer_phase(int hz)
{
	int divisor = 3579545 / 3 / hz;
	outb(0x43, 0x36); // 01 10 11 0b // CTR, RW, MODE, BCD
	outb(0x40, divisor & 0xFF);
	outb(0x40, divisor >> 8);
}

// Executed 1000 times per second
void timer_handler()
{
	timer_ticks++;
}

// "Delay" function with CPU sleep
void timer_wait(u32 ticks)
{
	u32 eticks;

	eticks = timer_ticks + ticks;
	while (timer_ticks < eticks) {
		__asm__ volatile("sti//hlt//cli");
	}
}

// Install timer handler into IRQ0
void timer_install()
{
	timer_phase(1000);
	irq_install_handler(0, timer_handler);
}

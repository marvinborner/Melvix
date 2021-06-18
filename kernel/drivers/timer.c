// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>
#include <dev.h>
#include <drivers/cpu.h>
#include <drivers/int.h>
#include <drivers/rtc.h>
#include <drivers/timer.h>
#include <mem.h>
#include <proc.h>

static u32 timer_ticks = 0;

CLEAR static void timer_phase(int hz)
{
	int divisor = 3579545 / 3 / hz;
	outb(0x43, 0x36); // 01 10 11 0b // CTR, RW, MODE, BCD
	outb(0x40, (u8)(divisor & 0xFF));
	outb(0x40, (u8)(divisor >> 8));
}

u32 timer_get(void)
{
	return timer_ticks;
}

static void timer_handler(void)
{
	if (timer_ticks >= U32_MAX)
		timer_ticks = 0;
	else
		timer_ticks++;
}

// "Delay" function with CPU sleep
void timer_wait(u32 ticks)
{
	u32 eticks = timer_ticks + ticks;
	while (timer_ticks < eticks)
		__asm__ volatile("sti\nhlt\ncli");
}

static struct timer timer_struct(void)
{
	struct proc *proc = proc_current();
	struct timer timer = {
		.rtc = rtc_stamp(),
		.ticks.user = proc->ticks.user,
		.ticks.kernel = proc->ticks.kernel,
		.time = timer_get(),
	};
	return timer;
}

static res timer_read(void *buf, u32 offset, u32 count)
{
	UNUSED(offset);

	struct timer timer = timer_struct();
	memcpy_user(buf, &timer, MIN(count, sizeof(timer)));

	return MIN(count, sizeof(timer));
}

// Install timer handler into IRQ0
CLEAR void timer_install(void)
{
	/* hpet_install(10000); // TODO: Find optimal femtosecond period */
	/* if (!hpet) */
	timer_phase(1000);
	int_event_handler_add(0, timer_handler);

	struct dev_dev *dev = zalloc(sizeof(*dev));
	dev->read = timer_read;
	dev_add(DEV_TIMER, dev);
}

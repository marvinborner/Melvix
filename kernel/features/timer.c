// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <def.h>
#include <dev.h>
#include <drivers/cpu.h>
#include <drivers/int.h>
#include <drivers/pit.h>
#include <drivers/rtc.h>
#include <mem.h>
#include <proc.h>
#include <timer.h>

static u32 timer_ticks = 0;

u32 timer_get(void)
{
	return timer_ticks;
}

static void timer_handler(void)
{
	int_disable();
	if (timer_ticks >= U32_MAX)
		timer_ticks = 0;
	else
		timer_ticks++;
	proc_timer_check(timer_ticks);
	int_enable();
}

CLEAR void timer_install_handler(void)
{
	static u8 check = 0;
	assert(check++ == 0);
	int_event_handler_add(0, timer_handler);
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

	// TODO: Make sleeping more accurate
	struct proc *proc = proc_current();
	if (proc->timer.mode == TIMER_MODE_SLEEP) {
		dev_block(DEV_TIMER, proc);
		proc->timer.mode = TIMER_MODE_DEFAULT;
	}

	struct timer timer = timer_struct();
	memcpy_user(buf, &timer, MIN(count, sizeof(timer)));

	return MIN(count, sizeof(timer));
}

static res timer_control(u32 request, void *arg1, void *arg2, void *arg3)
{
	UNUSED(arg2);
	UNUSED(arg3);

	switch (request) {
	case DEVCTL_TIMER_SLEEP: {
		struct proc *proc = proc_current();
		proc->timer.mode = TIMER_MODE_SLEEP;
		proc->timer.data = timer_ticks + (u32)arg1;
		return EOK;
	}
	default:
		return -EINVAL;
	}
}

static res timer_ready(void)
{
	struct proc *proc = proc_current();
	if (proc->timer.mode == TIMER_MODE_SLEEP) {
		if (timer_ticks >= proc->timer.data)
			return EOK;
		return -EAGAIN;
	}
	return EOK;
}

// Install timer handler into IRQ0
CLEAR void timer_install(void)
{
	// hpet_install(10000); // TODO: Reimplement hpet
	pit_install();

	struct dev_dev *dev = zalloc(sizeof(*dev));
	dev->read = timer_read;
	dev->control = timer_control;
	dev->ready = timer_ready;
	dev_add(DEV_TIMER, dev);
}

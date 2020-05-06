#include <stdint.h>
#include <interrupts/interrupts.h>
#include <io/io.h>
#include <system.h>

unsigned long timer_ticks = 0;

void timer_phase(int hz)
{
	int divisor = (int)(3579545.0 / 3.0 / (double)hz);
	outb(0x43, 0x36); // 01 10 11 0b // CTR, RW, MODE, BCD
	outb(0x40, (u8)(divisor & 0xFF));
	outb(0x40, (u8)(divisor >> 8));
}

// Executed 1000 times per second
void timer_handler(struct regs *r)
{
	timer_ticks++;
}

// "Delay" function with CPU sleep
void timer_wait(int ticks)
{
	u32 eticks;

	eticks = timer_ticks + ticks;
	while (timer_ticks < eticks) {
		asm("sti//hlt//cli");
	}
}

u32 get_time()
{
	return timer_ticks;
}

// Install timer handler into IRQ0
void timer_install()
{
	timer_phase(1000);
	irq_install_handler(0, timer_handler);
	info("Installed timer");
}
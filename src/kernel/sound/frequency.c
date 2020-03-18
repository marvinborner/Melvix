#include <stdint.h>
#include <kernel/io/io.h>
#include <kernel/timer/timer.h>

void play_sound(uint32_t frequency)
{
	uint32_t divided;
	uint8_t tmp;

	divided = 1193180 / frequency;
	outb(0x43, 0xb6);
	outb(0x42, (uint8_t)(divided));
	outb(0x42, (uint8_t)(divided >> 8));

	tmp = inb(0x61);
	if (tmp != (tmp | 3)) {
		outb(0x61, (uint8_t)(tmp | 3));
	}
}

static void shut_up()
{
	uint8_t tmp = (uint8_t)(inb(0x61) & 0xFC);

	outb(0x61, tmp);
}

// Make a beep
void beep(uint32_t frequency, uint32_t ticks)
{
	play_sound(frequency);
	timer_wait((int)ticks);
	shut_up();
}
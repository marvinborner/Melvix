#include <io/io.h>
#include <stdint.h>
#include <timer/timer.h>

void play_sound(u32 frequency)
{
	u32 divided;
	u8 tmp;

	divided = 1193180 / frequency;
	outb(0x43, 0xb6);
	outb(0x42, (u8)(divided));
	outb(0x42, (u8)(divided >> 8));

	tmp = inb(0x61);
	if (tmp != (tmp | 3)) {
		outb(0x61, (u8)(tmp | 3));
	}
}

static void shut_up()
{
	u8 tmp = (u8)(inb(0x61) & 0xFC);

	outb(0x61, tmp);
}

// Make a beep
void beep(u32 frequency, u32 ticks)
{
	play_sound(frequency);
	timer_wait((int)ticks);
	shut_up();
}
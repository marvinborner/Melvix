#include <syscall.h>

int is_transmit_empty()
{
	u8 value;
	asm volatile("inb %1, %0" : "=a"(value) : "Nd"(0x3f8 + 5));
	return value & 0x20;
}

void putch(char ch)
{
	while (is_transmit_empty() == 0)
		;
	asm volatile("outb %0, %1" ::"a"(ch), "Nd"(0x3f8));
}

/*void putch(char ch)
{
	// TODO: Implement framebuffer writing
	//if (ch != 0)
	//syscall_putch(ch);
}*/
// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>

u32 strlen(const char *s)
{
	const char *ss = s;
	while (*ss)
		ss++;
	return ss - s;
}

u8 inb(u16 port)
{
	u8 value;
	__asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
	return value;
}

void outb(u16 port, u8 data)
{
	__asm__ volatile("outb %0, %1" ::"a"(data), "Nd"(port));
}

int is_transmit_empty()
{
	return inb(0x3f8 + 5) & 0x20;
}

void serial_put(char ch)
{
	while (is_transmit_empty() == 0)
		;
	outb(0x3f8, (u8)ch);
}

void serial_print(const char *data)
{
	for (u32 i = 0; i < strlen(data); i++)
		serial_put(data[i]);
}

void main()
{
	serial_print("a");
}

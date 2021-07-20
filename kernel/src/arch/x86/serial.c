// MIT License, Copyright (c) 2021 Marvin Borner

#include <cpu.h>
#include <serial.h>

#define PORT 0x3f8

PROTECTED static u8 serial_enabled = 0;

CLEAR void serial_disable(void)
{
	outb(PORT + 4, 0x1e); // Enable loopback
	serial_enabled = 0;
}

CLEAR void serial_enable(void)
{
	outb(PORT + 4, 0x0f);
	serial_enabled = 1;
}

CLEAR u8 serial_init(void)
{
	outb(PORT + 1, 0x00);
	outb(PORT + 3, 0x80);
	outb(PORT + 0, 0x03);
	outb(PORT + 1, 0x00);
	outb(PORT + 3, 0x03);
	outb(PORT + 2, 0xc7);

	// Test serial chip
	outb(PORT + 4, 0x1e); // Enable loopback
	outb(PORT + 0, 0xae); // Write
	return inb(PORT + 0) != 0xae; // Verify receive
}

static int transmit_empty(void)
{
	return inb(PORT + 5) & 0x20;
}

static void serial_put(char ch)
{
	if (!serial_enabled)
		return;

	while (!transmit_empty())
		;
	outb(PORT, (u8)ch);
}

void serial_print(const char *data, size_t count)
{
	for (const char *p = data; *p && (size_t)(p - data) < count; p++)
		serial_put(*p);
}

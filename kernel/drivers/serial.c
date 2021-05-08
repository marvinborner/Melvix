// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <cpu.h>
#include <def.h>
#include <serial.h>
#include <str.h>

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

CLEAR void serial_install(void)
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
	assert(inb(PORT + 0) == 0xae); // Verify receive
}

static int is_transmit_empty(void)
{
	return inb(PORT + 5) & 0x20;
}

void serial_put(char ch)
{
	if (!serial_enabled)
		return;

	while (is_transmit_empty() == 0)
		;
	outb(PORT, (u8)ch);
}

void serial_print(const char *data)
{
	for (const char *p = data; p && *p; p++)
		serial_put(*p);
}

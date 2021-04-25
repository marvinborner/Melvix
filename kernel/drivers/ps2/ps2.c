// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <cpu.h>
#include <def.h>
#include <print.h>
#include <ps2.h>

#define PS2_TIMEOUT 100000

static struct ps2_status ps2_read_status(void)
{
	u8 byte = inb(0x64);
	return *(struct ps2_status *)&byte;
}

static u8 ps2_wait_readable(void)
{
	u32 time_out = PS2_TIMEOUT;
	while (time_out--)
		if (ps2_read_status().in_full)
			return 1;
	/* print("PS/2 readable timeout\n"); */
	return 0;
}

static u8 ps2_wait_writable(void)
{
	u32 time_out = PS2_TIMEOUT;
	while (time_out--)
		if (!ps2_read_status().out_full)
			return 1;
	/* print("PS/2 writable timeout\n"); */
	return 0;
}

u8 ps2_read_data(void)
{
	if (ps2_wait_readable()) {
		return inb(0x60);
	} else {
		return 0;
	}
}

u8 ps2_write_data(u8 byte)
{
	if (ps2_wait_writable()) {
		outb(0x60, byte);
		return 1;
	} else {
		return 0;
	}
}

CLEAR static u8 ps2_write_command(u8 byte)
{
	if (ps2_wait_writable()) {
		outb(0x64, byte);
		return 1;
	} else {
		return 0;
	}
}

CLEAR static struct ps2_config ps2_read_config(void)
{
	assert(ps2_write_command(0x20));
	u8 config = ps2_read_data();
	return *(struct ps2_config *)&config;
}

CLEAR static u8 ps2_write_config(struct ps2_config config)
{
	// Select first byte
	if (!ps2_write_command(0x60))
		return 0;

	return ps2_write_data(*(u8 *)&config);
}

PROTECTED static struct {
	u8 detected : 1;
	struct {
		u8 exists : 1;
		u16 type;
	} first;
	struct {
		u8 exists : 1;
		u16 type;
	} second;
} info = { 0 };

CLEAR u8 ps2_write_device(u8 device, u8 data)
{
	u8 resp = PS2_RESEND;
	for (u8 i = 0; resp == PS2_RESEND && i < 3; i++) {
		if (device == 1)
			ps2_write_command(0xd4);
		ps2_write_data(data);
		resp = ps2_read_data();
	}

	if (resp != PS2_ACK)
		return 0;

	return 1;
}

CLEAR static u8 ps2_device_keyboard(void)
{
	if (!info.detected)
		return U8_MAX;

	if (info.first.exists && PS2_KEYBOARD(info.first.type))
		return 0;
	else if (info.second.exists && PS2_KEYBOARD(info.second.type))
		return 1;

	return U8_MAX;
}

CLEAR static u8 ps2_device_mouse(void)
{
	if (!info.detected)
		return U8_MAX;

	if (info.first.exists && PS2_MOUSE(info.first.type))
		return 0;
	else if (info.second.exists && PS2_MOUSE(info.second.type))
		return 1;

	return U8_MAX;
}

CLEAR u8 ps2_keyboard_detect(void)
{
	if (!info.detected)
		return U8_MAX;

	u8 device = ps2_device_keyboard();
	if (device == U8_MAX)
		return device;

	ps2_write_device(device, 0xff);
	if (ps2_read_data() == 0xaa)
		return device;

	return U8_MAX;
}

CLEAR u8 ps2_mouse_detect(void)
{
	if (!info.detected)
		return U8_MAX;

	u8 device = ps2_device_mouse();
	if (device == U8_MAX)
		return device;

	ps2_write_device(device, 0xff);
	if (ps2_read_data() == 0xaa)
		return device;

	return U8_MAX;
}

CLEAR void ps2_detect(void)
{
	// TODO: Read ACPI 8042 flag in FADT to verify PS/2 support

	// Disable PS/2 ports
	ps2_write_command(0xad);
	ps2_write_command(0xa7);

	// Get config and disable IRQs
	struct ps2_config config = ps2_read_config();
	assert(!config.zero1 && !config.zero2);
	config.first_int = 0;
	config.second_int = 0;
	ps2_write_config(config);

	// Test PS/2 controller
	ps2_write_command(0xaa);
	if (ps2_read_data() != 0x55)
		return;
	ps2_write_config(config);

	// Test first PS/2 port
	ps2_write_command(0xab);
	if (ps2_read_data() == 0x0) {
		// Enable first port
		ps2_write_command(0xae);
		config.first_int = 1;
		config.first_clock_disabled = 0;
		info.first.exists = 1;
	}

	// Test and enable second PS/2 port if available
	if (config.second_clock_disabled) {
		ps2_write_command(0xa9);
		if (ps2_read_data() == 0x0) {
			// Enable second port
			ps2_write_command(0xa8);
			config.second_int = 1;
			config.second_clock_disabled = 0;
			info.second.exists = 1;
		}
	}

	ps2_write_config(config);

	// Detect device type of first port
	if (info.first.exists) {
		if (!ps2_write_device(0, 0xf5))
			return;

		if (!ps2_write_device(0, 0xf2))
			return;

		u8 first = ps2_read_data();
		u8 second = ps2_read_data();
		info.first.type = (first << 8) | second;
	}

	// Detect device type of second port
	if (info.second.exists) {
		if (!ps2_write_device(1, 0xf5))
			return;

		if (!ps2_write_device(1, 0xf2))
			return;

		u8 first = ps2_read_data();
		u8 second = ps2_read_data(); // This shall timeout if it's a mouse
		info.second.type = (first << 8) | second;
	}

	info.detected = 1;
}

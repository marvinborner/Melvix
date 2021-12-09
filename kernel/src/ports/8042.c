/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */
// 8042 is a PS/2 controller

#include <assert.h>
#include <print.h>

#include <core/io.h>
#include <ports/8042.h>

// Debugging mode
#define DEBUG panic
//#define DEBUG log

#define TIMEOUT 100

// Ports
#define PORT_DATA 0x60
#define PORT_STATUS 0x64
#define PORT_COMMAND 0x64

// Config
#define CONFIG_BYTE_READ 0x20
#define CONFIG_BYTE_WRITE 0x60

// Commands
#define ENABLE_FIRST 0xae
#define ENABLE_SECOND 0xa8
#define DISABLE_FIRST 0xad
#define DISABLE_SECOND 0xa7
#define SELECT_FIRST 0xd2
#define SELECT_SECOND 0xd4
#define TEST_FIRST_REQUEST 0xab
#define TEST_FIRST_RESPONSE 0x00
#define TEST_SECOND_REQUEST 0xa9
#define TEST_SECOND_RESPONSE 0x00
#define TEST_SELF_REQUEST 0xaa
#define TEST_SELF_RESPONSE 0x55

// Device
#define DEVICE_ACK 0xfa
#define DEVICE_RESEND 0xfe
#define DEVICE_DISABLE_SCANNING 0xf5
#define DEVICE_IDENTIFY 0xf2
#define DEVICE_TEST_REQUEST 0xff
#define DEVICE_TEST_RESPONSE 0xaa

// Status bits
#define STATUS_FULL_OUT 0
#define STATUS_FULL_IN 1
#define STATUS_SYSTEM 2
#define STATUS_COMMAND 3
#define STATUS_FULL_OUT2 5
#define STATUS_ERROR_TIME 6
#define STATUS_ERROR_PARITY 7

// Config bits
#define CONFIG_INTERRUPT_FIRST 0
#define CONFIG_INTERRUPT_SECOND 1
#define CONFIG_RUNNING 2
#define CONFIG_ZERO1 3
#define CONFIG_CLOCK_DISABLED_FIRST 4
#define CONFIG_CLOCK_DISABLED_SECOND 5
#define CONFIG_TRANSLATION_FIRST 6
#define CONFIG_ZERO2 7

static u8 read_status(void)
{
	return IO_INB(PORT_STATUS);
}

static u8 wait_readable(void)
{
	u32 time_out = TIMEOUT;
	while (time_out--)
		if (BIT_GET(read_status(), STATUS_FULL_OUT) ||
		    !BIT_GET(read_status(), STATUS_FULL_OUT2)) // TODO: Better
			return 1;
	DEBUG("Timeout readable");
	return 0;
}

static u8 wait_writable(void)
{
	u32 time_out = TIMEOUT;
	while (time_out--)
		if (!BIT_GET(read_status(), STATUS_FULL_IN))
			return 1;
	DEBUG("Timeout writable");
	return 0;
}

static void write_command(u8 byte)
{
	if (wait_readable())
		IO_OUTB(PORT_COMMAND, byte);
	else
		DEBUG("Write command 0x%x failed", byte);
}

static void select_byte(u8 byte)
{
	write_command(byte);
}

static u8 read_data(void)
{
	if (wait_readable())
		return IO_INB(PORT_DATA);
	DEBUG("Read data failed");
	return 0;
}

static void write_data(u8 byte)
{
	if (wait_writable())
		IO_OUTB(PORT_DATA, byte);
	else
		DEBUG("Write 0x%x data failed", byte);
}

static void write_device(u8 device, u8 data)
{
	if (device != 0 && device != 1)
		DEBUG("Invalid device %d", device);

	u8 response;
	for (u8 i = 0; i < 5; i++) {
		if (device == 1)
			write_command(SELECT_SECOND);
		write_data(data);
		response = read_data();
		if (response == DEVICE_ACK)
			return;
	}

	DEBUG("Write 0x%x to %d failed: 0x%x", data, device, response);
}

static u8 read_config(void)
{
	select_byte(CONFIG_BYTE_READ);
	return read_data();
}

static void write_config(u8 config)
{
	select_byte(CONFIG_BYTE_WRITE);
	write_data(config);
}

static void disable(void)
{
	// Disable both ports
	write_command(DISABLE_FIRST);
	write_command(DISABLE_SECOND);

	// Get config and disable IRQs
	u8 config = read_config();
	config = BIT_CLEAR(config, CONFIG_INTERRUPT_FIRST);
	config = BIT_CLEAR(config, CONFIG_INTERRUPT_SECOND);
	write_config(config);
}

static u8 test_self(void)
{
	write_command(TEST_SELF_REQUEST);
	return read_data() == TEST_SELF_RESPONSE;
}

static u8 test_first(void)
{
	write_command(TEST_FIRST_REQUEST);
	return read_data() == TEST_FIRST_RESPONSE;
}

static u8 test_second(void)
{
	write_command(TEST_SECOND_REQUEST);
	return read_data() == TEST_SECOND_RESPONSE;
}

static u16 device_type(u8 device)
{
	static u16 first = 0, second = 0;

	if (device == 0 && first)
		return first;
	else if (device == 1 && second)
		return second;

	write_device(device, DEVICE_DISABLE_SCANNING);
	write_device(device, DEVICE_IDENTIFY);
	u16 type = (read_data() << 8) | read_data();
	if (device == 0)
		first = type;
	else if (device == 1)
		second = type;
	return type;
}

static err device_test(u8 device)
{
	write_device(device, DEVICE_TEST_REQUEST);
	return read_data() == DEVICE_TEST_RESPONSE ? ERR_OK : -ERR_HARDWARE;
}

static err read(void *buf, u32 offset, u32 count)
{
	if (count != 1 || offset)
		return -ERR_INVALID_ARGUMENTS; // lol
	*(u8 *)buf = read_data();
	return ERR_OK;
}

static err request(u32 request, va_list ap)
{
	UNUSED(ap);

	switch (request) {
	case PORT_8042_DEVICE_TYPE:
		return device_type(va_arg(ap, int));
	case PORT_8042_DEVICE_TEST:
		return device_test(va_arg(ap, int));
	case PORT_8042_DEVICE_WRITE: {
		u8 device = va_arg(ap, int);
		u8 data = va_arg(ap, int);
		write_device(device, data);
		return ERR_OK;
	}
	default:
		return -ERR_INVALID_ARGUMENTS;
	}
}

// TODO: Support for single-port controllers
static err probe(void)
{
	static err done = 0xff;
	if (done != 0xff)
		return -done;

	disable();

	// Tests may reset config
	u8 config = read_config();

	u8 self = test_self();
	write_config(config);
	u8 first = test_first();
	u8 second = test_second();
	write_config(config);

	done = (self && first && second) ? ERR_OK : ERR_HARDWARE;
	return -done;
}

static err setup(void)
{
	static u8 done = 0;
	if (done)
		return ERR_OK;

	u8 config = read_config();

	write_command(ENABLE_FIRST);
	config = BIT_SET(config, CONFIG_INTERRUPT_FIRST);
	config = BIT_CLEAR(config, CONFIG_CLOCK_DISABLED_FIRST);

	write_command(ENABLE_SECOND);
	config = BIT_SET(config, CONFIG_INTERRUPT_SECOND);
	config = BIT_CLEAR(config, CONFIG_CLOCK_DISABLED_SECOND);

	write_config(config);

	done = 1;
	return ERR_OK;
}

PROTECTED struct port port_8042 = {
	.type = PORT_8042,
	.read = read,
	.request = request,
	.probe = probe,
	.setup = setup,
};

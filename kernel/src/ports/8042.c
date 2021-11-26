/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */
// 8042 is a PS/2 controller

#include <assert.h>
#include <print.h>

#include <core/io.h>
#include <ports/8042.h>

#define TIMEOUT 100

#define DATA_PORT 0x60
#define STATUS_PORT 0x64
#define COMMAND_PORT 0x64

#define CONFIG_BYTE_READ 0x20
#define CONFIG_BYTE_WRITE 0x60

static struct ps2_status read_status(void)
{
	u8 byte = IO_INB(STATUS_PORT);
	return *(struct ps2_status *)&byte;
}

static u8 wait_readable(void)
{
	u32 time_out = TIMEOUT;
	while (time_out--)
		if (read_status().in_full)
			return 1;
	log("PS/2 readable timeout");
	return 0;
}

static u8 wait_writable(void)
{
	u32 time_out = TIMEOUT;
	while (time_out--)
		if (read_status().out_full)
			return 1;
	log("PS/2 writable timeout");
	return 0;
}

static u8 select_byte(u8 byte)
{
	if (wait_readable()) {
		IO_OUTB(COMMAND_PORT, byte);
		return 1;
	} else {
		return 0;
	}
}

static u8 read_data(void)
{
	if (wait_readable())
		return IO_INB(DATA_PORT);
	else
		return 0;
}

static u8 write_data(u8 byte)
{
	if (wait_readable()) {
		IO_OUTB(DATA_PORT, byte);
		return 1;
	} else {
		return 0;
	}
}

static struct ps2_config read_config(void)
{
	assert(select_byte(CONFIG_BYTE_READ));
	u8 config = read_data();
	return *(struct ps2_config *)&config;
}

static u8 write_config(struct ps2_config config)
{
	if (!select_byte(CONFIG_BYTE_WRITE))
		return 0;
	return write_data(*(u8 *)&config);
}

PROTECTED struct port port_8042 = {
	.type = PORT_8250,
};

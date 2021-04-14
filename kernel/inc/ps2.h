// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef PS2_H
#define PS2_H

#include <def.h>

#define PS2_ACK 0xfa
#define PS2_RESEND 0xfe

struct ps2_status {
	u8 in_full : 1;
	u8 out_full : 1;
	u8 system : 1;
	u8 command : 1;
	u8 reserved : 2;
	u8 error_time : 1;
	u8 error_parity : 1;
};

struct ps2_config {
	u8 first_int : 1;
	u8 second_int : 1;
	u8 running : 1;
	u8 zero1 : 1;
	u8 first_clock_disabled : 1;
	u8 second_clock_disabled : 1;
	u8 first_translation : 1;
	u8 zero2 : 1;
};

u8 ps2_read_data(void);
u8 ps2_write_data(u8 byte);
struct ps2_status ps2_read_status(void);
u8 ps2_write_command(u8 byte);

void ps2_detect(void);
u8 ps2_keyboard_support(void);
u8 ps2_mouse_support(void);

void ps2_keyboard_install(void);
void ps2_keyboard_reset(void);

void ps2_mouse_install(void);

#endif

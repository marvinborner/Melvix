// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef PS2_H
#define PS2_H

#include <def.h>

#define PS2_ACK 0xfa
#define PS2_RESEND 0xfe

#define PS2_TYPE_STANDARD_MOUSE 0x0000
#define PS2_TYPE_WHEEL_MOUSE 0x0003
#define PS2_TYPE_BUTTON_MOUSE 0x0004
#define PS2_TYPE_TRANSLATION_KEYBOARD1 0xab41
#define PS2_TYPE_TRANSLATION_KEYBOARD2 0xabc1
#define PS2_TYPE_STANDARD_KEYBOARD 0xab83

#define PS2_KEYBOARD(type)                                                                         \
	((type) == PS2_TYPE_TRANSLATION_KEYBOARD1 || (type) == PS2_TYPE_TRANSLATION_KEYBOARD2 ||   \
	 (type) == PS2_TYPE_STANDARD_KEYBOARD)
#define PS2_MOUSE(type)                                                                            \
	((type) == PS2_TYPE_STANDARD_MOUSE || (type) == PS2_TYPE_WHEEL_MOUSE ||                    \
	 (type) == PS2_TYPE_BUTTON_MOUSE)

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
u8 ps2_write_device(u8 device, u8 data);

void ps2_detect(void);
u8 ps2_keyboard_detect(void);
u8 ps2_mouse_detect(void);
void ps2_mouse_enable(u8 device);
void ps2_mouse_install(u8 device);

void ps2_keyboard_reset(void);
void ps2_keyboard_install(u8 device);

#endif

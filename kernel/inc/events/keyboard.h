/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#ifndef EVENTS_KEYBOARD_H
#define EVENTS_KEYBOARD_H

#include <err.h>

#define KEY_TO_CHAR(key) ((key) >= KEY_0 && (key) <= KEY_Z ? (key) + '0' : '?')

enum keyboard_key {
	KEY_0,
	KEY_1,
	KEY_2,
	KEY_3,
	KEY_4,
	KEY_5,
	KEY_6,
	KEY_7,
	KEY_8,
	KEY_9,
	KEY_a,
	KEY_b,
	KEY_c,
	KEY_d,
	KEY_e,
	KEY_f,
	KEY_g,
	KEY_h,
	KEY_i,
	KEY_j,
	KEY_k,
	KEY_l,
	KEY_m,
	KEY_n,
	KEY_o,
	KEY_p,
	KEY_q,
	KEY_r,
	KEY_s,
	KEY_t,
	KEY_u,
	KEY_v,
	KEY_w,
	KEY_x,
	KEY_y,
	KEY_z,
	KEY_A,
	KEY_B,
	KEY_C,
	KEY_D,
	KEY_E,
	KEY_F,
	KEY_G,
	KEY_H,
	KEY_I,
	KEY_J,
	KEY_K,
	KEY_L,
	KEY_M,
	KEY_N,
	KEY_O,
	KEY_P,
	KEY_Q,
	KEY_R,
	KEY_S,
	KEY_T,
	KEY_U,
	KEY_V,
	KEY_W,
	KEY_X,
	KEY_Y,
	KEY_Z,
	KEY_CTRL_L,
	KEY_CTRL_R,
	KEY_ALT_L,
	KEY_ALT_R,
	KEY_TAB,
	KEY_ENTER,
	KEY_BACKSPACE,
	KEY_DELETE,
	KEY_ESCAPE,
	KEY_LEFT,
	KEY_UP,
	KEY_RIGHT,
	KEY_DOWN,
	// TODO: More keys
};

struct keyboard_data {
	enum keyboard_key key;
	u8 pressed : 1;
};

NONNULL err keyboard_handler(struct keyboard_data *data);

#endif

/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#ifndef PORTS_8042_H
#define PORTS_8042_H

#include <management/port/index.h>

extern struct port port_8042;

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

#endif

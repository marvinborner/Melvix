/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#ifndef PORTS_8042_H
#define PORTS_8042_H

#include <management/port/index.h>

#define PORT_8042_DEVICE_TYPE 1
#define PORT_8042_DEVICE_TEST 2
#define PORT_8042_DEVICE_WRITE 3

extern struct port port_8042;

#endif

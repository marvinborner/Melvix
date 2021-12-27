/**
 * Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
 * SPDX-License-Identifier: MIT
 */

#ifndef DRIVERS_TIMER_H
#define DRIVERS_TIMER_H

#include <management/device/index.h>

#define DEVICE_TIMER_GET 1
#define DEVICE_TIMER_PHASE 2 // in ms // TODO: Smaller unit

extern struct device device_timer;

#endif

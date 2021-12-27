# Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
# SPDX-License-Identifier: MIT

VERSION = v0.0

CONFIG_CROSS_PATH ?= $(PWD)/cross
CONFIG_CACHE ?= ccache

DRIVER_SERIAL ?= ./src/drivers/serial/8250.c
DRIVER_TIMER ?= ./src/drivers/timer/8253.c
DRIVER_INTERRUPT ?= ./src/drivers/interrupt/8259.c
DRIVER_KEYBOARD ?= ./src/drivers/keyboard/8042.c
DRIVER_MOUSE ?= ./src/drivers/mouse/8042.c
# DRIVER_NETWORK ?= ./src/drivers/network/rtl8139.c
# DRIVER_VIDEO ?= ./src/drivers/video/bga.c

# Specific config groups
ifeq ($(CONFIG), debug)
	CONFIG_OPTIMIZE ?= fast
	CONFIG_CFLAGS ?= -Wno-error -ggdb3 -s -fsanitize=undefined -fstack-protector-all
else ifeq ($(CONFIG), dev)
	CONFIG_OPTIMIZE ?= fast
	CONFIG_CFLAGS ?= -finline -finline-functions
else ifeq ($(CONFIG), release)
endif

CONFIG_OPTIMIZE ?= fast
CONFIG_CFLAGS ?=

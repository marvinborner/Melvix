# MIT License, Copyright (c) 2021 Marvin Borner

# All preprocessor flags - enable using the custom config group below
ALL_PREPROCESSOR_FLAGS = \
    DEBUG_ALLOC \
    DEBUG_SYSCALLS \
    DEBUG_SCHEDULER

# All config options
ALL_CONFIGS = \
	CONFIG_STRIP \
	CONFIG_CACHE \
	CONFIG_EXTRA_CFLAGS \
	CONFIG_USER_PIE

# Set ccache globally
CONFIG_CACHE ?= ccache

# Specific config groups
ifeq ($(CONFIG), debug)
    CONFIG_OPTIMIZATION ?= -Ofast
    CONFIG_EXTRA_CFLAGS ?= -Wno-error -ggdb3 -s -fsanitize=undefined -fstack-protector-all
else ifeq ($(CONFIG), dev)
    CONFIG_OPTIMIZATION ?= -finline -finline-functions -Ofast
else ifeq ($(CONFIG), release)
    CONFIG_OPTIMIZATION ?= -finline -finline-functions -Ofast
    CONFIG_STRIP ?= true
else ifeq ($(CONFIG), experimental)
    CONFIG_USER_PIE ?= true
else ifeq ($(CONFIG), custom)
    DEBUG_ALLOC ?= true
    DEBUG_SYSCALLS ?= true
    DEBUG_SCHEDULER ?= true
endif

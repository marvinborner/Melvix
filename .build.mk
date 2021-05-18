# MIT License, Copyright (c) 2021 Marvin Borner

# All preprocessor flags - enable using the custom config group below
ALL_PREPROCESSOR_FLAGS = \
    DEBUG_ALLOC \
    DEBUG_SCHEDULER

# All config options
ALL_CONFIGS = \
	CONFIG_CACHE \
	CONFIG_EXTRA_CFLAGS \
	CONFIG_USER_PIE

# Specific config groups
ifeq ($(CONFIG), debug)
    CONFIG_OPTIMIZATION ?= -Ofast
    CONFIG_EXTRA_CFLAGS ?= -Wno-error -ggdb3 -s -fsanitize=undefined -fstack-protector-all
    CONFIG_CACHE ?= ccache
else ifeq ($(CONFIG), dev)
    CONFIG_OPTIMIZATION ?= -finline -finline-functions -Ofast
    CONFIG_CACHE ?= ccache
else ifeq ($(CONFIG), release)
    CONFIG_OPTIMIZATION ?= -finline -finline-functions -Ofast
    CONFIG_STRIP ?= true
    CONFIG_CACHE ?= ccache
else ifeq ($(CONFIG), custom)
    DEBUG_ALLOC ?= true
    DEBUG_SCHEDULER ?= true
endif

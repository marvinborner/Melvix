# MIT License, Copyright (c) 2020 Marvin Borner

CONFIG ?= dev
include .build.mk

define PREPROCESSOR_FLAG_TEMPLATE =
-D$(1)=$(if $(filter $($(1)),true),1,0)
endef

# Tools
CC = $(CONFIG_CACHE) $(PWD)/cross/opt/bin/i686-elf-gcc
LD = $(CONFIG_CACHE) $(PWD)/cross/opt/bin/i686-elf-ld
OC = $(CONFIG_CACHE) $(PWD)/cross/opt/bin/i686-elf-objcopy
ST = $(CONFIG_CACHE) $(PWD)/cross/opt/bin/i686-elf-strip
AS = $(CONFIG_CACHE) nasm

# Flags
PREPROCESSOR_FLAGS = $(foreach flag, $(ALL_PREPROCESSOR_FLAGS), $(call PREPROCESSOR_FLAG_TEMPLATE,$(flag)))
CFLAGS_WARNINGS = -Wall -Wextra -Werror -Wshadow -Wpointer-arith -Wwrite-strings -Wredundant-decls -Wnested-externs -Wformat=2 -Wmissing-declarations -Wstrict-prototypes -Wmissing-prototypes -Wcast-qual -Wswitch-default -Wswitch-enum -Wunreachable-code -Wundef -Wold-style-definition -Wvla -pedantic-errors
CFLAGS = $(CFLAGS_WARNINGS) -std=c99 -m32 -nostdlib -nostdinc -fno-builtin -fno-profile-generate -fno-omit-frame-pointer -fno-common -fno-asynchronous-unwind-tables -mno-red-zone -mno-mmx -mno-sse -mno-sse2 $(CONFIG_OPTIMIZATION) $(CONFIG_EXTRA_CFLAGS) $(PREPROCESSOR_FLAGS)
ASFLAGS = -f elf32

# Directories
BUILD = $(PWD)/build
KERNEL = $(PWD)/kernel
LIBS = $(PWD)/libs

export

all: dir $(LIBS) $(KERNEL)

dir:
	@mkdir -p $(BUILD)

$(LIBS):
	@echo "[MK] Libraries"
	@$(MAKE) --no-print-directory -C $@

$(KERNEL): $(LIBS)
	@echo "[MK] Kernel"
	@$(MAKE) --no-print-directory -C $@

clean:
	@rm -rf $(BUILD)/*

.PHONY: all dir $(LIBS) $(KERNEL)

# MIT License, Copyright (c) 2020 Marvin Borner

CONFIG ?= dev
include .build.mk

define PREPROCESSOR_FLAG_TEMPLATE =
-D$(1)=$(if $(filter $($(1)),true),1,0)
endef

PREPROCESSOR_FLAGS = $(foreach flag, $(ALL_PREPROCESSOR_FLAGS), $(call PREPROCESSOR_FLAG_TEMPLATE,$(flag)))

CFLAGS_WARNINGS = -Wall -Wextra -Werror -Wshadow -Wpointer-arith -Wwrite-strings -Wredundant-decls -Wnested-externs -Wformat=2 -Wmissing-declarations -Wstrict-prototypes -Wmissing-prototypes -Wcast-qual -Wswitch-default -Wswitch-enum -Wunreachable-code -Wundef -Wold-style-definition -Wvla -Winline -pedantic-errors
CFLAGS_DEFAULT = $(CFLAGS_WARNINGS) -std=c99 -m32 -nostdlib -nostdinc -fno-builtin -fno-profile-generate -fno-omit-frame-pointer -fno-common -fno-asynchronous-unwind-tables -mno-red-zone -mno-80387 -mno-mmx -mno-sse -mno-sse2 $(CONFIG_OPTIMIZATION) $(CONFIG_EXTRA_CFLAGS) $(PREPROCESSOR_FLAGS)

CC = $(CONFIG_CACHE) $(PWD)/cross/opt/bin/i686-elf-gcc
LD = $(CONFIG_CACHE) $(PWD)/cross/opt/bin/i686-elf-ld
OC = $(CONFIG_CACHE) $(PWD)/cross/opt/bin/i686-elf-objcopy
ST = $(CONFIG_CACHE) $(PWD)/cross/opt/bin/i686-elf-strip
AS = $(CONFIG_CACHE) nasm

BUILD = $(PWD)/build/
KERNEL = $(PWD)/kernel/
LIBS = $(PWD)/libs/

all: compile

export

compile:
	@echo Using '$(CONFIG)' config
	@$(MAKE) clean --no-print-directory -C libs/libc/
	@$(MAKE) libc --no-print-directory -C libs/libc/
	@echo "Compiled libc"
	@$(MAKE) clean --no-print-directory -C libs/libc/
	@$(MAKE) libk --no-print-directory -C libs/libc/
	@echo "Compiled libk"
	@$(MAKE) --no-print-directory -C libs/libgui/
	@echo "Compiled libgui"
	@$(MAKE) --no-print-directory -C libs/libtxt/
	@echo "Compiled libtxt"
	@$(MAKE) --no-print-directory -C kernel/
	@echo "Compiled kernel"
	@$(MAKE) --no-print-directory -C apps/
	@echo "Compiled apps"

clean:
	@find kernel/ apps/ libs/ boot/ \( -name "*.o" -or -name "*.a" -or -name "*.elf" -or -name "*.bin" \) -type f -delete

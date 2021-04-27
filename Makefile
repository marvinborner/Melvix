# MIT License, Copyright (c) 2020 Marvin Borner

CFLAGS_OPTIMIZATION = -finline -finline-functions -Ofast
CFLAGS_WARNINGS = -Wall -Wextra -Werror -Wshadow -Wpointer-arith -Wwrite-strings -Wredundant-decls -Wnested-externs -Wformat=2 -Wmissing-declarations -Wstrict-prototypes -Wmissing-prototypes -Wcast-qual -Wswitch-default -Wswitch-enum -Wlogical-op -Wunreachable-code -Wundef -Wold-style-definition -Wvla -pedantic-errors
CFLAGS_DEFAULT = $(CFLAGS_WARNINGS) $(CFLAGS_OPTIMIZATION) -std=c99 -m32 -nostdlib -nostdinc -fno-builtin -fno-profile-generate -fno-omit-frame-pointer -fno-common -fno-asynchronous-unwind-tables -mno-red-zone -mno-80387 -mno-mmx -mno-sse -mno-sse2

CC = ccache $(PWD)/cross/opt/bin/i686-elf-gcc
LD = ccache $(PWD)/cross/opt/bin/i686-elf-ld
OC = ccache $(PWD)/cross/opt/bin/i686-elf-objcopy
ST = ccache $(PWD)/cross/opt/bin/i686-elf-strip
AS = ccache nasm

BUILD = $(PWD)/build/
KERNEL = $(PWD)/kernel/
LIBS = $(PWD)/libs/

ifeq ($(DEBUG), 1)
	CFLAGS_DEFAULT += -Wno-error -ggdb3 -s -fsanitize=undefined -fstack-protector-all
endif

all: compile

export

compile:
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
	@$(MAKE) --no-print-directory -C boot/
	@echo "Compiled boot"
	@$(MAKE) --no-print-directory -C apps/
	@echo "Compiled apps"

clean:
	@find kernel/ apps/ libs/ boot/ \( -name "*.o" -or -name "*.a" -or -name "*.elf" -or -name "*.bin" \) -type f -delete

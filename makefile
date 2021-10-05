# Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
# SPDX-License-Identifier: MIT

MELVIX_CONFIG ?= dev # Env var
CONFIG = $(MELVIX_CONFIG)
include config.mk

CC = $(CONFIG_CACHE) $(CONFIG_CROSS_PATH)/bin/i686-elf-gcc
LD = $(CONFIG_CACHE) $(CONFIG_CROSS_PATH)/bin/i686-elf-ld
OC = $(CONFIG_CACHE) $(CONFIG_CROSS_PATH)/bin/i686-elf-objcopy
ST = $(CONFIG_CACHE) $(CONFIG_CROSS_PATH)/bin/i686-elf-strip
AS = $(CONFIG_CACHE) nasm
SU = sudo
TG = ctags

CFLAGS_WARNINGS = -Wall -Wextra -Werror -Wshadow -Wpointer-arith -Wwrite-strings -Wredundant-decls -Wnested-externs -Wformat=2 -Wmissing-declarations -Wstrict-prototypes -Wmissing-prototypes -Wcast-qual -Wswitch-default -Wswitch-enum -Wunreachable-code -Wundef -Wold-style-definition -Wvla -pedantic-errors
CFLAGS = $(CFLAGS_WARNINGS) -std=c99 -m32 -nostdlib -nostdinc -fno-builtin -fno-profile-generate -fno-omit-frame-pointer -fno-common -fno-asynchronous-unwind-tables -ffreestanding -mno-red-zone -mno-mmx -mno-sse -mno-sse2 $(CONFIG_CFLAGS) -O$(CONFIG_OPTIMIZE)
ASFLAGS = -f elf32

BUILD = $(PWD)/build
KERNEL = $(PWD)/kernel
LIBS = $(PWD)/libs

export

all: $(LIBS) $(KERNEL)

compile: all sync

clean:
	@rm -rf $(BUILD)/*

cross:
	@./cross.sh

qemu: compile $(BUILD) $(BUILD)/disk.img
	@qemu-system-i386 -d guest_errors,unimp,pcall -cpu max -no-reboot -vga std -m 256M -enable-kvm -serial stdio -kernel $(BUILD)/kernel.elf -drive file=$(BUILD)/disk.img,format=raw,index=1,media=disk

sync: # Ugly hack
	@$(MAKE) --always-make --dry-run | grep -wE 'gcc' | grep -w '\-c' | jq -nR '[inputs|{directory: match("/melvix/build/[^.]+").string[14:], command: ., file: match(" [^ ]+$$").string[1:]}]' >compile_commands.json &
	@$(TG) -R --exclude=.git --exclude=build --exclude=cross . &

$(LIBS): $(BUILD)
	@echo "[MK] libs"
	@$(MAKE) --no-print-directory -C $@

$(KERNEL): $(LIBS) $(BUILD)
	@echo "[MK] kernel"
	@$(MAKE) --no-print-directory -C $@

.PHONY: all compile clean cross dir qemu sync $(KERNEL) $(LIBS)

$(BUILD):
	@mkdir -p $@

%.img: $(BUILD)
	@dd if=/dev/zero of=$@ bs=1k count=32k status=none
	@DEV=$$($(SU) losetup --find --partscan --show $@) ;\
	PART="p1";\
	$(SU) parted -s "$$DEV" mklabel msdos mkpart primary ext2 32k 100% -a minimal set 1 boot on; \
	$(SU) mke2fs -b 1024 -q "$$DEV$$PART"; \
	$(SU) losetup -d "$$DEV"; \

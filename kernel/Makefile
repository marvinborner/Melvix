# MIT License, Copyright (c) 2020 Marvin Borner

COBJS = main.o \
	drivers/acpi.o \
	drivers/interrupts.o \
	drivers/interrupts_asm.o \
	drivers/fpu.o \
	drivers/keyboard.o \
	drivers/mouse.o \
	drivers/pci.o \
	drivers/ide.o \
	drivers/timer.o \
	drivers/rtl8139.o \
	features/fs.o \
	features/load.o \
	features/proc.o \
	features/proc_asm.o \
	features/syscall.o \
	features/net.o \
	features/event.o
CC = ccache ../cross/opt/bin/i686-elf-gcc
LD = ccache ../cross/opt/bin/i686-elf-ld
OC = ccache ../cross/opt/bin/i686-elf-objcopy
AS = ccache nasm

WARNINGS = -Wall -Wextra -pedantic-errors -Wshadow -Wpointer-arith -Wwrite-strings -Wredundant-decls -Wnested-externs -Wno-long-long
CFLAGS = $(WARNINGS) -Wno-address-of-packed-member -nostdlib -nostdinc -ffreestanding -fno-builtin -fno-profile-generate -mno-red-zone -mgeneral-regs-only -std=c99 -m32 -Wl,-ekernel_main -I../libc/inc/ -Iinc/ -Dkernel $(CFLAGS_EXTRA) $(OPTIMIZATION) $(DEBUG)
ASFLAGS = -f elf32

all: compile

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@

%_asm.o: %.asm
	@$(AS) $(ASFLAGS) $< -o $@

compile: $(COBJS)
	@mkdir -p ../build/
	@$(LD) -N -ekernel_main -Ttext 0x00050000 -o ../build/kernel.elf -L../build/ $+ -lk
	@$(OC) -O binary ../build/kernel.elf ../build/kernel.bin

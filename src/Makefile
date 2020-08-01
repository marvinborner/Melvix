# MIT License, Copyright (c) 2020 Marvin Borner

COBJS = main.o \
		drivers/vesa.o \
		drivers/cpu.o \
		drivers/serial.o \
		drivers/interrupts.o \
		drivers/interrupts_asm.o \
		drivers/keyboard.o \
		drivers/ide.o \
		drivers/timer.o \
		features/fs.o \
		features/psf.o \
		features/gui.o \
		features/load.o \
		lib/str.o \
		lib/mem.o \
		lib/math.o \
		lib/conv.o \
		lib/print.o
CC = ../cross/opt/bin/i686-elf-gcc
LD = ../cross/opt/bin/i686-elf-ld
OC = ../cross/opt/bin/i686-elf-objcopy
AS = nasm

# Flags to make the binary smaller TODO: Remove after indirect pointer support!
CSFLAGS = -mpreferred-stack-boundary=2 -fno-asynchronous-unwind-tables -Os

# TODO: Use lib as external library
CFLAGS = $(CSFLAGS) -Wall -Wextra -nostdlib -nostdinc -ffreestanding -fno-builtin -mgeneral-regs-only -std=c99 -m32 -pedantic-errors -Wl,-ekernel_main -Ilib/inc/ -Iinc/

ASFLAGS = -f elf32 -O3

all: compile

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@

%_asm.o: %.asm
	@$(AS) $(ASFLAGS) $< -o $@

compile: $(COBJS)
	@mkdir -p ../build/
	@$(AS) -f bin entry.asm -o ../build/boot.bin
	@$(LD) -N -ekernel_main -Ttext 0x00050000 -o ../build/kernel.bin $(COBJS) --oformat binary
	@$(CC) $(CFLAGS) -o ../build/debug.o $(COBJS)

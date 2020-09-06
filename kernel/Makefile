# MIT License, Copyright (c) 2020 Marvin Borner

COBJS = main.o \
	drivers/acpi.o \
	drivers/interrupts.o \
	drivers/interrupts_asm.o \
	drivers/fpu.o \
	drivers/keyboard.o \
	drivers/mouse.o \
	drivers/ide.o \
	drivers/timer.o \
	features/fs.o \
	features/load.o \
	features/proc.o \
	features/proc_asm.o \
	features/syscall.o \
	features/event.o
CC = ../cross/opt/bin/i686-elf-gcc
LD = ../cross/opt/bin/i686-elf-ld
OC = ../cross/opt/bin/i686-elf-objcopy
AS = nasm

# Flags to make the binary smaller TODO: Remove after indirect pointer support!
CSFLAGS = -mpreferred-stack-boundary=2 -fno-asynchronous-unwind-tables -Os

CFLAGS = $(CSFLAGS) -Wall -Wextra -nostdlib -nostdinc -ffreestanding -fno-builtin -mno-red-zone -mgeneral-regs-only -std=c99 -m32 -pedantic-errors -Wl,-ekernel_main -I../libc/inc/ -Iinc/ -Dkernel

ASFLAGS = -f elf32

all: compile bootloader

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@

%_asm.o: %.asm
	@$(AS) $(ASFLAGS) $< -o $@

bootloader:
	@mkdir -p ../build/
	@$(AS) -f bin entry.asm -o ../build/boot.bin

compile: $(COBJS)
	@mkdir -p ../build/
	@$(LD) -N -ekernel_main -Ttext 0x00050000 -o ../build/kernel.bin -L../build/ $+ -lk --oformat binary
	@$(CC) $(CFLAGS) -o ../build/debug.o -L../build/ $+ -lk

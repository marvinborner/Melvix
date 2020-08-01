# MIT License, Copyright (c) 2020 Marvin Borner

COBJS_KERNEL = src/main.o \
		src/drivers/vesa.o \
		src/drivers/cpu.o \
		src/drivers/serial.o \
		src/drivers/interrupts.o \
		src/drivers/interrupts_asm.o \
		src/drivers/keyboard.o \
		src/drivers/ide.o \
		src/drivers/timer.o \
		src/features/fs.o \
		src/features/psf.o \
		src/features/gui.o \
		src/features/load.o \
		src/lib/str.o \
		src/lib/mem.o \
		src/lib/math.o \
		src/lib/conv.o \
		src/lib/print.o
CC = cross/opt/bin/i686-elf-gcc
LD = cross/opt/bin/i686-elf-ld
OC = cross/opt/bin/i686-elf-objcopy
AS = nasm

# Flags to make the binary smaller TODO: Remove after indirect pointer support!
CSFLAGS = -mpreferred-stack-boundary=2 -fno-asynchronous-unwind-tables -Os

# TODO: Use lib as external library
CFLAGS = $(CSFLAGS) -Wall -Wextra -nostdlib -nostdinc -ffreestanding -fno-builtin -mgeneral-regs-only -std=c99 -m32 -pedantic-errors -Isrc/lib/inc/ -Isrc/inc/

ASFLAGS = -f elf32 -O3

all: compile clean

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@

%_asm.o: %.asm
	@$(AS) $(ASFLAGS) $< -o $@

kernel: $(COBJS_KERNEL)

compile: kernel
	@mkdir -p build/
	@$(AS) -f bin src/entry.asm -o build/boot.bin
	@$(LD) -N -emain -Ttext 0x00050000 -o build/kernel.bin $(COBJS_KERNEL) --oformat binary
	@$(CC) $(CFLAGS) -o build/debug.o $(COBJS_KERNEL)
	@$(CC) $(CFLAGS) -fPIE -c apps/test.c -o build/test.o
	@$(LD) -o build/test.elf -Tapps/link.ld build/test.o
	@$(OC) -O binary build/test.elf build/test

clean:
	@find src/ apps/ -name "*.o" -type f -delete

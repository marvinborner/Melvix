# MIT License, Copyright (c) 2020 Marvin Borner

COBJS = src/main.o \
		src/drivers/vesa.o \
		src/drivers/cpu.o \
		src/drivers/serial.o \
		src/drivers/interrupts.o \
		src/drivers/interrupts_asm.o \
		src/drivers/keyboard.o \
		src/drivers/ide.o \
		src/features/fs.o \
		src/features/psf.o \
		src/features/gui.o \
		src/lib/str.o \
		src/lib/mem.o \
		src/lib/math.o \
		src/lib/conv.o \
		src/lib/print.o
CC = cross/opt/bin/i686-elf-gcc
LD = cross/opt/bin/i686-elf-ld
AS = nasm

# TODO: Use lib as external library
CFLAGS = -Wall -Wextra -nostdlib -nostdinc -ffreestanding -fno-builtin -fno-pic -mgeneral-regs-only -std=c99 -m32 -pedantic-errors -Isrc/lib/inc/ -Isrc/inc/ -c

ASFLAGS = -f elf32

all: compile clean

%.o: %.c
	@$(CC) $(CFLAGS) $< -o $@

%_asm.o: %.asm
	@$(AS) $(ASFLAGS) $< -o $@

kernel: $(COBJS)

compile: kernel
	@mkdir -p build/
	@$(AS) -f bin src/entry.asm -o build/boot.bin
	@$(LD) -N -emain -Ttext 0x00050000 -o build/kernel.bin $(COBJS) --oformat binary

clean:
	@find src/ -name "*.o" -type f -delete

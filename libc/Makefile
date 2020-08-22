# MIT License, Copyright (c) 2020 Marvin Borner

# TODO: Remove serial and cpu from libc?
COBJS = str.o \
	mem.o \
	math.o \
	conv.o \
	print.o \
	serial.o \
	cpu.o \
	sys.o \
	list.o \
	random.o
CC = ../cross/opt/bin/i686-elf-gcc
LD = ../cross/opt/bin/i686-elf-ld
AR = ../cross/opt/bin/i686-elf-ar
AS = nasm

# Flags to make the binary smaller TODO: Remove after indirect pointer support!
CSFLAGS = -mpreferred-stack-boundary=2 -fno-asynchronous-unwind-tables -Os

CFLAGS = $(CSFLAGS) -Wall -Wextra -nostdlib -nostdinc -fno-builtin -mno-red-zone -mgeneral-regs-only -std=c99 -m32 -pedantic-errors -Iinc/

ASFLAGS = -f elf32

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@

libc: CFLAGS += -Duserspace -fPIE
libc: $(COBJS)
	@$(AS) $(ASFLAGS) crt/crt0.asm -o crt0.o
	@mkdir -p ../build/
	@$(AR) rcs ../build/libc.a crt0.o $+

libk: CFLAGS += -Dkernel -ffreestanding
libk: $(COBJS)
	@mkdir -p ../build/
	@$(AR) rcs ../build/libk.a $+

clean:
	@find . -name "*.o" -type f -delete

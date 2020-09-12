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

CFLAGS = -Wall -Wextra -nostdlib -nostdinc -fno-builtin -fno-asynchronous-unwind-tables -mno-red-zone -mgeneral-regs-only -mpreferred-stack-boundary=2 -std=c99 -m32 -pedantic-errors -Iinc/ -Os

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

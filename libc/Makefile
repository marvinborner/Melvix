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
	list.o
CC = ../cross/opt/bin/i686-elf-gcc
LD = ../cross/opt/bin/i686-elf-ld
OC = ../cross/opt/bin/i686-elf-ar

# Flags to make the binary smaller TODO: Remove after indirect pointer support!
CSFLAGS = -mpreferred-stack-boundary=2 -fno-asynchronous-unwind-tables -Os

CFLAGS = $(CSFLAGS) -Wall -Wextra -nostdlib -nostdinc -ffreestanding -fno-builtin -mgeneral-regs-only -std=c99 -m32 -pedantic-errors -Iinc/

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@

libc: CFLAGS += -Duserspace -fPIE
libc: $(COBJS)
	@mkdir -p ../build/
	@$(AR) qc ../build/libc.a $+

libk: CFLAGS += -Dkernel
libk: $(COBJS)
	@mkdir -p ../build/
	@$(AR) qc ../build/libk.a $+

clean:
	@find . -name "*.o" -type f -delete

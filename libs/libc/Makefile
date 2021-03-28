# MIT License, Copyright (c) 2020 Marvin Borner

# TODO: Remove cpu from libc?
COBJS = sanitize.o \
	errno.o \
	str.o \
	alloc.o \
	mem.o \
	math.o \
	crypto.o \
	conv.o \
	print.o \
	cpu.o \
	sys.o \
	list.o \
	stack.o \
	random.o
CC = ccache ../../cross/opt/bin/i686-elf-gcc
LD = ccache ../../cross/opt/bin/i686-elf-ld
AR = ccache ../../cross/opt/bin/i686-elf-ar
AS = ccache nasm

CFLAGS = $(CFLAGS_DEFAULT) -Iinc/

ASFLAGS = -f elf32

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@

libc: CFLAGS += -Duserspace
libc: $(COBJS)
	@$(AS) $(ASFLAGS) crt/crt0.asm -o crt0.o
	@mkdir -p ../../build/
	@$(AR) rcs ../../build/libc.a crt0.o $+

libk: CFLAGS += -Dkernel -ffreestanding -I../../kernel/inc/ $(CFLAGS_EXTRA)
libk: $(COBJS)
	@mkdir -p ../../build/
	@$(AR) rcs ../../build/libk.a $+

clean:
	@find . -name "*.o" -type f -delete
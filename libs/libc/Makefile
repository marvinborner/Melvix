# MIT License, Copyright (c) 2020 Marvin Borner

# TODO: Remove cpu from libc?
COBJS = crt/crt0.o \
	sanitize.o \
	errno.o \
	str.o \
	alloc.o \
	mem.o \
	math.o \
	crypto.o \
	conv.o \
	print.o \
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

libc: CFLAGS += -pie -fPIE -fPIC -DUSER
libc: $(COBJS)
	@mkdir -p ../../build/
	@$(AS) $(ASFLAGS) crt/crti.asm -o ../../build/crti.o
	@$(AS) $(ASFLAGS) crt/crtn.asm -o ../../build/crtn.o
	@$(AR) rcs ../../build/libc.a ../../build/crti.o ../../build/crti.o $+

libk: CFLAGS += -DKERNEL -ffreestanding -I../../kernel/inc/ $(CFLAGS_EXTRA)
libk: $(COBJS)
	@mkdir -p ../../build/
	@$(AR) rcs ../../build/libk.a $+

clean:
	@find . -name "*.o" -type f -delete

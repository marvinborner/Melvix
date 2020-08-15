# MIT License, Copyright (c) 2020 Marvin Borner

COBJS = vesa.o
CC = ../cross/opt/bin/i686-elf-gcc
LD = ../cross/opt/bin/i686-elf-ld
OC = ../cross/opt/bin/i686-elf-ar

# Flags to make the binary smaller TODO: Remove after indirect pointer support!
CSFLAGS = -mpreferred-stack-boundary=2 -fno-asynchronous-unwind-tables -Os

CFLAGS = $(CSFLAGS) -Wall -Wextra -nostdlib -nostdinc -ffreestanding -fno-builtin -mgeneral-regs-only -std=c99 -m32 -pedantic-errors -Iinc/ -I../libc/inc/ -Duserspace -fPIE

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@

libgui: $(COBJS)
	@mkdir -p ../build/
	@$(AR) qc ../build/libgui.a $+

clean:
	@find . -name "*.o" -type f -delete

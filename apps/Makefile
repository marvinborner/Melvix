# MIT License, Copyright (c) 2020 Marvin Borner

COBJS = test.o
CC = ../cross/opt/bin/i686-elf-gcc
LD = ../cross/opt/bin/i686-elf-ld
OC = ../cross/opt/bin/i686-elf-objcopy

# TODO: Fix crash without optimizations
CSFLAGS = -mpreferred-stack-boundary=2 -fno-asynchronous-unwind-tables -Os

CFLAGS = $(CSFLAGS) -Wall -Wextra -nostdlib -nostdinc -ffreestanding -fno-builtin -mgeneral-regs-only -std=c99 -m32 -pedantic-errors -I../src/lib/inc/ -fPIE -Os

all: $(COBJS)

%.o: %.c
	@mkdir -p ../build/
	@$(CC) -c $(CFLAGS) $< -o $@
	@$(LD) -o $(@:.o=.elf) -Tlink.ld $@
	@$(OC) -O binary $(@:.o=.elf) ../build/$(@:.o=)

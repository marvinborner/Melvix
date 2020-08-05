# MIT License, Copyright (c) 2020 Marvin Borner

COBJS = a.o b.o init.o
CC = ../cross/opt/bin/i686-elf-gcc
LD = ../cross/opt/bin/i686-elf-ld
OC = ../cross/opt/bin/i686-elf-objcopy

CFLAGS = $(CSFLAGS) -Wall -Wextra -nostdlib -nostdinc -ffreestanding -ffunction-sections -fno-builtin -std=c99 -m32 -pedantic-errors -I../src/lib/inc/ -fPIE

all: $(COBJS)

%.o: %.c
	@mkdir -p ../build/apps/
	@$(CC) -c $(CFLAGS) $< -o $@
	@$(LD) -o $(@:.o=.elf) -Tlink.ld $@
	@$(OC) -O binary $(@:.o=.elf) ../build/apps/$(@:.o=)

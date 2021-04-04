# MIT License, Copyright (c) 2020 Marvin Borner

CC = ccache ../cross/opt/bin/i686-elf-gcc
LD = ccache ../cross/opt/bin/i686-elf-ld
OC = ccache ../cross/opt/bin/i686-elf-objcopy
AS = ccache nasm

CFLAGS = $(CFLAGS_DEFAULT) -ffreestanding -fno-stack-protector -fno-sanitize=undefined -Ofast

ASFLAGS = -f elf32

all: compile

compile:
	@mkdir -p ../build/
	@$(CC) -c $(CFLAGS) load.c -o load.o
	@$(LD) -N -emain -Ttext 0x00009000 -o ../build/load.bin load.o --oformat binary
	@$(AS) -f bin entry.asm -o ../build/boot.bin

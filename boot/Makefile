# MIT License, Copyright (c) 2020 Marvin Borner

CFLAGS = $(CFLAGS_DEFAULT) -ffreestanding -fno-stack-protector -fno-sanitize=undefined -Ofast

ASFLAGS = -f elf32

all: compile

compile:
	@mkdir -p $(BUILD)
	@$(CC) -c $(CFLAGS) load.c -o load.o
	@$(LD) -N -emain -Ttext 0x00009000 -o $(BUILD)/load.bin load.o --oformat binary
	@$(AS) -f bin entry.asm -o $(BUILD)/boot.bin

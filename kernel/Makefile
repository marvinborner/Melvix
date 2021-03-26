# MIT License, Copyright (c) 2020 Marvin Borner
# acpi.o fpu.o
COBJS = main.o \
	drivers/interrupts.o \
	drivers/interrupts_asm.o \
	drivers/serial.o \
	drivers/keyboard.o \
	drivers/mouse.o \
	drivers/pci.o \
	drivers/ide.o \
	drivers/fb.o \
	drivers/timer.o \
	drivers/rtl8139.o \
	features/mm.o \
	features/fs.o \
	features/load.o \
	features/proc.o \
	features/proc_asm.o \
	features/syscall.o \
	features/net.o
CC = ccache ../cross/opt/bin/i686-elf-gcc
LD = ccache ../cross/opt/bin/i686-elf-ld
OC = ccache ../cross/opt/bin/i686-elf-objcopy
AS = ccache nasm

CFLAGS = $(CFLAGS_DEFAULT) -Wno-address-of-packed-member -ffreestanding -Wl,-ekernel_main -I../libs/libc/inc/ -Iinc/ -Dkernel
ASFLAGS = -f elf32

all: compile

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@

%_asm.o: %.asm
	@$(AS) $(ASFLAGS) $< -o $@

compile: $(COBJS)
	@mkdir -p ../build/
	@$(LD) -N -z undefs -ekernel_main -Ttext 0x00009000 -o ../build/kernel.elf -L../build/ $+ -lk
	@$(LD) -N -z max-page-size=0x1000 -Tlink.ld -o ../build/kernel.bin -L../build/ $+ -lk

# MIT License, Copyright (c) 2020 Marvin Borner
# acpi.o fpu.o
COBJS = main.o \
	drivers/interrupts.o \
	drivers/interrupts_asm.o \
	drivers/cpu.o \
	drivers/serial.o \
	drivers/keyboard.o \
	drivers/mouse.o \
	drivers/pci.o \
	drivers/rtc.o \
	drivers/ide.o \
	drivers/fb.o \
	drivers/timer.o \
	features/mm.o \
	features/fs.o \
	features/load.o \
	features/proc.o \
	features/proc_asm.o \
	features/syscall.o
CC = ccache ../cross/opt/bin/i686-elf-gcc
LD = ccache ../cross/opt/bin/i686-elf-ld
OC = ccache ../cross/opt/bin/i686-elf-objcopy
AS = ccache nasm

CFLAGS = $(CFLAGS_DEFAULT) -Wno-address-of-packed-member -ffreestanding -Wl,-ekernel_main -I../libs/libc/inc/ -Iinc/ -DKERNEL
ASFLAGS = -f elf32

all: compile

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@

%_asm.o: %.asm
	@$(AS) $(ASFLAGS) $< -o $@

compile: $(COBJS)
	@mkdir -p ../build/apps/
	@$(LD) -N -z max-page-size=0x1000 -ekernel_main -Tlink.ld -o ../build/apps/kernel -L../build/ $+ -lk

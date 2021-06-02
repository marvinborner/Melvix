# MIT License, Copyright (c) 2020 Marvin Borner
# acpi.o fpu.o
COBJS = entry_asm.o \
	main.o \
	multiboot.o \
	drivers/int.o \
	drivers/int_asm.o \
	drivers/pic.o \
	drivers/gdt.o \
	drivers/cpu.o \
	drivers/serial.o \
	drivers/pci.o \
	drivers/rtc.o \
	drivers/mbr.o \
	drivers/ide.o \
	drivers/vbe.o \
	drivers/bga.o \
	drivers/timer.o \
	drivers/vmware.o \
	drivers/ps2/ps2.o \
	drivers/ps2/mouse.o \
	drivers/ps2/keyboard.o \
	features/mm.o \
	features/fs.o \
	features/io.o \
	features/bus.o \
	features/fb.o \
	features/logger.o \
	features/load.o \
	features/proc.o \
	features/proc_asm.o \
	features/syscall.o

CFLAGS = $(CFLAGS_DEFAULT) -Wno-address-of-packed-member -ffreestanding -Wl,-ekernel_main -I$(LIBS)/libc/inc/ -Iinc/ -DKERNEL
ASFLAGS = -f elf32

all: compile

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@

%_asm.o: %.asm
	@$(AS) $(ASFLAGS) $< -o $@

compile: $(COBJS)
	@mkdir -p $(BUILD)/apps/kernel/
	@$(LD) -N -z max-page-size=0x1000 -eboot_entry -Tlink.ld -o $(BUILD)/apps/kernel/exec -L$(BUILD) $+ -lk
ifeq ($(CONFIG_STRIP), true)
	@$(ST) --strip-all $(BUILD)/apps/kernel/exec
endif

COBJS = src/main.o \
		src/drivers/vesa.o
CC = cross/opt/bin/i686-elf-gcc
LD = cross/opt/bin/i686-elf-ld
AS = nasm

CFLAGS = -Wall -Wextra -nostdlib -nostdinc -ffreestanding -std=c99 -pedantic-errors -Isrc/lib/ -Isrc/inc/ -c

all: compile clean

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

kernel: $(COBJS)

compile: kernel
	mkdir -p build/
	$(AS) -f bin src/entry.asm -o build/boot.bin
	$(LD) -N -emain -Ttext 0x00050000 -o build/kernel.bin $(COBJS) --oformat binary

clean:
	find src/ -name "*.o" -type f -delete

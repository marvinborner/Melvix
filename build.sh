#!/usr/bin/env sh
# Builds the operating system image

# Install and source cross compiler if not already installed
. ./cross.sh

# Create build directories
rm -rf ./build/ ./iso/
mkdir ./build/

# Make source files
i686-elf-as ./src/boot.s -o ./build/boot.o
i686-elf-gcc -c ./src/kernel.c -o ./build/kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
i686-elf-gcc -T ./src/linker.ld -o ./build/melvix.bin -ffreestanding -O2 -nostdlib ./build/boot.o ./build/kernel.o -lgcc

# Testing
if grub-file --is-x86-multiboot ./build/melvix.bin; then
  echo Multiboot confirmed
else
  echo Melvix has errors and won\'t be able to multi boot!
  exit
fi

# Create ISO
mkdir -p ./iso/boot/grub
cp ./build/melvix.bin ./iso/boot/
cp ./src/grub.cfg ./iso/boot/grub/
grub-mkrescue -o ./build/melvix.iso ./iso/

# Run ISO
qemu-system-x86_64 -cdrom ./build/melvix.iso

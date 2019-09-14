#!/usr/bin/env sh
# Builds the operating system image

# Install and source cross compiler if not already installed
. ./cross.sh

# Create build directories
rm -rf ./build/ ./iso/
mkdir ./build/

# Make source files
i686-elf-as ./src/boot.s -o ./build/boot.o

files=""
find ./src -name \*.c >./build/tmp
while read -r line; do
  stripped=$(echo "${line}" | sed -r 's/\//_/g')
  stripped=${stripped#??????}
  stripped=${stripped%%?}o
  i686-elf-gcc -c ./"${line}" -o ./build/"${stripped}" -std=gnu99 -ffreestanding -O2 -Wall -Wextra
  files="${files} ./build/${stripped}"
done <./build/tmp
rm ./build/tmp

# shellcheck disable=SC2086
# Shellcheck suppression is needed because gcc would think that $files is one file
i686-elf-gcc -T ./src/linker.ld -o ./build/melvix.bin -ffreestanding -O2 -nostdlib ./build/boot.o $files -lgcc

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

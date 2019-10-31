.DEFAULT_GOAL := build
.SILENT: cross build test

DIR := $(shell pwd)/cross
export PREFIX := $(DIR)/opt
export TARGET := i686-elf
export PATH := $(PREFIX)/bin:$(PATH)

clean:
	@-rm -rf ./build ./iso
	@echo Cleaned build directory

build: clean
	@set -e; \
	mkdir ./build/; \

	# Assemble ASM files
	nasm -f elf ./src/kernel/boot.asm -o ./build/boot.o || exit; \

	# Make all C files
	find ./src/kernel/ ./src/userspace/ -name \*.c >./build/tmp; \
	while read -r line; do \
		stripped=$$(echo "$${line}" | sed -r 's/\//_/g'); \
		stripped=$${stripped#??????}; \
		stripped=$${stripped%%?}o; \
		i686-elf-gcc -c ./"$${line}" -o ./build/"$${stripped}" -I ./src -std=gnu99 -ffreestanding -O3 -Wall -Wextra -Wno-unused-parameter || exit; \
	done <./build/tmp; \
	rm ./build/tmp; \

	i686-elf-gcc -T ./src/kernel/linker.ld -I ./src -o ./build/melvix.bin -std=gnu99 -ffreestanding -O2 -nostdlib ./build/*.o || exit; \

	# Testing
	if grub-file --is-x86-multiboot ./build/melvix.bin; then \
		echo Multiboot confirmed; \
	else \
		echo Melvix has errors and won\'t be able to multi boot!; \
		exit; \
	fi; \

	# Create ISO
	mkdir -p ./iso/boot/grub; \
	cp ./build/melvix.bin ./iso/boot/; \
	cp ./src/bootloader/grub.cfg ./iso/boot/grub/; \
	gcc ./src/bootloader/make_initrd.c -o ./build/make_initrd || exit; \
	./build/make_initrd ./src/bootloader/test.txt test.txt || exit; \
	mv initrd.img ./iso/boot/melvix.initrd || exit; \
	grub-mkrescue -o ./build/melvix.iso ./iso/;

cross:
	@set -e; \
	[ -d "./cross/" ] && echo "Please remove ./cross/ and try again" && exit; \
	mkdir cross || exit; \
	cd cross || exit; \
	DIR=$$(pwd); \
	mkdir "$${DIR}/src" && cd "$${DIR}/src" || exit; \
	echo "Downloading..."; \
	curl -sSL "https://ftp.gnu.org/gnu/binutils/binutils-2.32.tar.xz" | tar xJ; \
	curl -sSL "https://ftp.gnu.org/gnu/gcc/gcc-9.2.0/gcc-9.2.0.tar.xz" | tar xJ; \
	mkdir -p "$${DIR}/opt/bin"; \
	export PREFIX="$${DIR}/opt"; \
	export TARGET=i686-elf; \
	export PATH="$$PREFIX/bin:$$PATH"; \
	mkdir "$${DIR}/src/build-binutils" && cd "$${DIR}/src/build-binutils" || exit; \
	../binutils-2.32/configure --target="$$TARGET" --prefix="$$PREFIX" --with-sysroot --disable-nls --disable-werror; \
	make; \
	make install; \
	mkdir "$${DIR}/src/build-gcc" && cd "$${DIR}/src/build-gcc" || exit; \
	../gcc-9.2.0/configure --target="$$TARGET" --prefix="$$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers; \
	make all-gcc; \
	make all-target-libgcc; \
	make install-gcc; \
	make install-target-libgcc; \
	cd "$${DIR}/.." || exit;

test: build debug

debug:
	@rm -f qemu.log
	@echo "Starting simulation"
	@echo "[SERIAL OUTPUT]"
	@qemu-system-x86_64 -no-reboot -soundhw pcspk -M accel=kvm:tcg -vga std -serial stdio -rtc base=localtime -d cpu_reset -D qemu.log -m 512M -cdrom ./build/melvix.iso
	@echo "[END OF CONNECTION]"

.PHONY: build clean cross test debug
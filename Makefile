.DEFAULT_GOAL := build
.SILENT: cross build test

DIR := $(shell pwd)/cross
export PREFIX := $(DIR)/opt
export TARGET := i686-elf
export PATH := $(PREFIX)/bin:$(PATH)
export NETWORK := rtl8139

clean:
	@-rm -rf ./build ./iso
	@echo "Cleaned build directory"

build: clean
	@set -e; \
	echo "Building..."; \
	mkdir -p ./build/kernel && mkdir -p ./build/userspace; \

	# Assemble ASM files
	nasm -f elf ./src/kernel/boot.asm -o ./build/kernel/boot.o || exit; \

	# Make all kernel C files
	find ./src/kernel/ -name \*.c >./build/tmp; \
	while read -r line; do \
		stripped=$$(echo "$${line}" | sed -r 's/\//_/g'); \
		stripped=$${stripped#??????}; \
		stripped=$${stripped%%?}o; \
		i686-elf-gcc -c ./"$${line}" -o ./build/kernel/"$${stripped}" -I ./src -std=gnu99 -ffreestanding -O3 -Wall -Wextra -Wno-unused-parameter -D ${NETWORK} || exit; \
	done <./build/tmp; \
	rm ./build/tmp; \
	i686-elf-gcc -T ./src/kernel/linker.ld -I ./src -o ./build/melvix.bin -std=gnu99 -ffreestanding -O2 -nostdlib ./build/kernel/*.o || exit; \

	# Modules
	i686-elf-gcc -c ./src/resources/font.c -o ./build/font.o -I ./src -std=gnu99 -ffreestanding -O2 -nostdlib; \
	i686-elf-objcopy -O binary ./build/font.o ./build/font.bin; \
	rm ./build/font.o; \

	# Userspace
	nasm -f elf ./src/userspace/start.asm -o ./build/userspace/start.o || exit; \
	find ./src/userspace/ -name \*.c >./build/tmp; \
	while read -r line; do \
		stripped=$$(echo "$${line}" | sed -r 's/\//_/g'); \
		stripped=$${stripped#??????}; \
		stripped=$${stripped%%?}o; \
		i686-elf-gcc -c ./"$${line}" -o ./build/userspace/"$${stripped}" -I ./src/userspace -std=gnu99 -ffreestanding -O3 -Wall -Wextra -Wno-unused-parameter || exit; \
	done <./build/tmp; \
	rm ./build/tmp; \
	i686-elf-gcc -I ./src/userspace -o ./build/user.o -std=gnu99 -ffreestanding -O2 -nostdlib ./build/userspace/*.o|| exit; \
	i686-elf-objcopy -O binary ./build/user.o ./build/user.bin; \

	# Create ISO
	mkdir -p ./iso/boot/; \
	mv ./build/melvix.bin ./iso/boot/kernel.bin; \
	nasm ./src/bootloader/cd.asm -f bin -o ./iso/boot/cd.bin || exit; \
	nasm ./src/bootloader/hdd1.asm -f bin -o ./iso/boot/hdd1.bin || exit; \
	nasm ./src/bootloader/hdd2.asm -f bin -o ./iso/boot/hdd2.bin || exit; \
	cp ./build/user.bin ./iso/user.bin || exit; \
	cp ./build/font.bin ./iso/font.bin || exit; \
	genisoimage -quiet -input-charset utf-8 -no-emul-boot -b boot/cd.bin -o ./build/melvix.iso ./iso;

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

QEMU_OPTIONS := -no-reboot -vga std -smp $$(nproc) -serial stdio -rtc base=localtime -m 256M -net nic,model=rtl8139,macaddr=42:42:42:42:42:42 -net user

debug:
	@echo "Starting simulation..."
	@head -c 10485760 /dev/zero > ./build/hdd10M.img
	@echo "[SERIAL OUTPUT]"
	@qemu-system-x86_64 ${QEMU_OPTIONS} -cdrom ./build/melvix.iso -drive file=./build/hdd10M.img,format=raw
	@echo "[END OF CONNECTION]"

image: build
	@echo "Starting simulation..."
	@head -c 10485760 /dev/zero > ./build/hdd10M.img
	@echo "[SERIAL OUTPUT]"
	@qemu-system-x86_64 ${QEMU_OPTIONS} -cdrom ./build/melvix.iso -drive file=./build/hdd10M.img,format=raw
	@echo "[END OF CONNECTION]"
	@printf "\n"
	@echo "[SERIAL OUTPUT]"
	@qemu-system-x86_64 ${QEMU_OPTIONS} -drive file=./build/hdd10M.img,format=raw
	@echo "[END OF CONNECTION]"

debugHDD:
	@echo "Starting simulation..."
	@echo "[SERIAL OUTPUT]"
	@qemu-system-x86_64 ${QEMU_OPTIONS} -drive file=./build/hdd10M.img,format=raw
	@echo "[END OF CONNECTION]"

bochs: build
	@head -c 10485760 /dev/zero > ./build/hdd10M.img
	@qemu-system-x86_64 ${QEMU_OPTIONS} -cdrom ./build/melvix.iso -drive file=./build/hdd10M.img,format=raw
	@bochs -f bochs.txt

bochsHDD:
	@bochs -f bochs.txt

.PHONY: build clean cross test debug image debugHDD bochs bochsHDD
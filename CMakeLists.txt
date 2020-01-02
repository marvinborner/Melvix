cmake_minimum_required(VERSION 3.0)
project(Melvix)
enable_language(ASM)

set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS " ")
set(CMAKE_C_COMPILER "${CMAKE_CURRENT_SOURCE_DIR}/cross/opt/bin/i686-elf-gcc")
set(CMAKE_ASM_COMPILER "nasm")
set(CMAKE_LINKER "${CMAKE_CURRENT_SOURCE_DIR}/cross/opt/bin/i686-elf-gcc")

set(CMAKE_C_FLAGS "-c -I./src -std=gnu99 -ffreestanding -O2 -Wall -Wextra")
set(CMAKE_ASM_COMPILE_OBJECT "<CMAKE_ASM_COMPILER> <SOURCE> -o <OBJECT> -f elf32")
set(CMAKE_EXE_LINKER_FLAGS "-T ${CMAKE_CURRENT_SOURCE_DIR}/src/kernel/linker.ld -o ${CMAKE_CURRENT_SOURCE_DIR}/iso/boot/kernel.bin -ffreestanding -O2 -nostdlib")
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_LINKER> ${CMAKE_EXE_LINKER_FLAGS} <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

file(GLOB_RECURSE kernel_sources src/kernel/*.c src/kernel/*.h src/kernel/*.asm)
file(GLOB_RECURSE resources_sources src/resources/*.c)
file(GLOB_RECURSE user_sources src/userspace/*.c src/userspace/*.h src/userspace/*.asm)

# KERNEL
include_directories(kernel "./src")
add_executable(kernel ${kernel_sources})
set_target_properties(kernel PROPERTIES OUTPUT_NAME "${CMAKE_CURRENT_SOURCE_DIR}/iso/boot/kernel.bin")

# RESOURCES
include_directories(resources "./src")
add_executable(resources ${resources_sources})
set_target_properties(resources PROPERTIES OUTPUT_NAME "${CMAKE_CURRENT_SOURCE_DIR}/iso/font.o")
add_custom_command(
        TARGET resources POST_BUILD
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/cross/opt/bin/i686-elf-objcopy -O binary ${CMAKE_CURRENT_SOURCE_DIR}/iso/font.o ${CMAKE_CURRENT_SOURCE_DIR}/iso/font.bin
        COMMAND rm ${CMAKE_CURRENT_SOURCE_DIR}/iso/font.o
)

# USERSPACE
include_directories(user "./src/userspace")
add_executable(user ${user_sources})
set_target_properties(user PROPERTIES OUTPUT_NAME "${CMAKE_CURRENT_SOURCE_DIR}/iso/user.o")
add_custom_command(
        TARGET user POST_BUILD
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/cross/opt/bin/i686-elf-objcopy -O binary ${CMAKE_CURRENT_SOURCE_DIR}/iso/user.o ${CMAKE_CURRENT_SOURCE_DIR}/iso/user.bin
        COMMAND rm ${CMAKE_CURRENT_SOURCE_DIR}/iso/user.o
)

# ISO
add_custom_target(
        iso
        COMMAND nasm ${CMAKE_CURRENT_SOURCE_DIR}/src/bootloader/cd.asm -o ${CMAKE_CURRENT_SOURCE_DIR}/iso/boot/cd.bin -f bin
        COMMAND nasm ${CMAKE_CURRENT_SOURCE_DIR}/src/bootloader/hdd1.asm -o ${CMAKE_CURRENT_SOURCE_DIR}/iso/boot/hdd1.bin -f bin
        COMMAND nasm ${CMAKE_CURRENT_SOURCE_DIR}/src/bootloader/hdd2.asm -o ${CMAKE_CURRENT_SOURCE_DIR}/iso/boot/hdd2.bin -f bin
        COMMAND genisoimage -input-charset utf-8 -no-emul-boot -b boot/cd.bin -o ${CMAKE_CURRENT_SOURCE_DIR}/iso/melvix.iso ${CMAKE_CURRENT_SOURCE_DIR}/iso
)

# TEST
add_custom_target(
        test
        COMMAND head -c 10485760 /dev/zero > ${CMAKE_CURRENT_SOURCE_DIR}/iso/hdd10M.img
        COMMAND qemu-system-i386 -no-reboot -vga std -smp $$(nproc) -serial stdio -rtc base=localtime -m 256M -net nic,model=rtl8139,macaddr=42:42:42:42:42:42 -net user -cdrom ${CMAKE_CURRENT_SOURCE_DIR}/melvix.iso -drive file=${CMAKE_CURRENT_SOURCE_DIR}/iso/hdd10M.img,format=raw
)

add_custom_target(clean_iso COMMAND rm -rf ${CMAKE_CURRENT_SOURCE_DIR}/iso && mkdir -p ${CMAKE_CURRENT_SOURCE_DIR}/iso/boot)
add_dependencies(kernel clean_iso resources user)
add_dependencies(iso kernel)
add_dependencies(test iso)

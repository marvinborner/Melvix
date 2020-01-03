cmake_minimum_required(VERSION 3.0)
project(Melvix)
enable_language(ASM)
set(CMAKE_VERBOSE_MAKEFILE on)

# Melvix variables
set(NETWORK "rtl8139")

# Compiler and linker
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS " ")
set(CMAKE_C_COMPILER "${CMAKE_CURRENT_SOURCE_DIR}/cross/opt/bin/i686-elf-gcc")
set(CMAKE_ASM_COMPILER "nasm")
set(CMAKE_LINKER "${CMAKE_CURRENT_SOURCE_DIR}/cross/opt/bin/i686-elf-gcc")
set(CMAKE_ASM_LINKER_PREFERENCE ${CMAKE_LINKER})
set(CMAKE_C_LINKER_PREFERENCE ${CMAKE_LINKER})

# Compiler and linker flags
set(CMAKE_C_FLAGS "-c -std=gnu99 -ffreestanding -O2 -Wall -Wextra")
set(CMAKE_EXE_LINKER_FLAGS "-std=gnu99 -ffreestanding -O3 -nostdlib")
set(CMAKE_ASM_COMPILE_OBJECT "<CMAKE_ASM_COMPILER> <SOURCE> -o <OBJECT> -f elf")
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_LINKER> ${CMAKE_EXE_LINKER_FLAGS} <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_ASM_LINK_EXECUTABLE "<CMAKE_LINKER> ${CMAKE_EXE_LINKER_FLAGS} <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

# Recursive sources
file(GLOB_RECURSE kernel_sources src/kernel/*.c src/kernel/*.asm)
file(GLOB_RECURSE resources_sources src/resources/*.c)
file(GLOB_RECURSE user_sources src/userspace/*.c src/userspace/*.asm)

# KERNEL
add_executable(kernel ${kernel_sources})
target_include_directories(kernel PRIVATE "src")
set_target_properties(kernel PROPERTIES OUTPUT_NAME "${CMAKE_CURRENT_SOURCE_DIR}/iso/boot/kernel.bin")
target_compile_options(kernel PRIVATE "-D ${NETWORK}")
target_link_libraries(kernel PRIVATE "-T ${CMAKE_CURRENT_SOURCE_DIR}/src/kernel/linker.ld")

# RESOURCES
add_executable(resources ${resources_sources})
set_target_properties(resources PROPERTIES OUTPUT_NAME "${CMAKE_CURRENT_SOURCE_DIR}/iso/font.o")
add_custom_command(
        TARGET resources POST_BUILD
        COMMAND cross/opt/bin/i686-elf-objcopy -O binary iso/font.o iso/font.bin
        COMMAND rm iso/font.o
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# USERSPACE
add_executable(user ${user_sources})
target_include_directories(user PRIVATE "src/userspace")
set_target_properties(user PROPERTIES OUTPUT_NAME "${CMAKE_CURRENT_SOURCE_DIR}/iso/user.o")
add_custom_command(
        TARGET user POST_BUILD
        COMMAND cross/opt/bin/i686-elf-objcopy -O binary iso/user.o iso/user.bin
        COMMAND rm iso/user.o
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# ISO
add_custom_command(
        TARGET kernel POST_BUILD
        COMMAND nasm src/bootloader/cd.asm -o iso/boot/cd.bin -f bin
        COMMAND nasm src/bootloader/hdd1.asm -o iso/boot/hdd1.bin -f bin
        COMMAND nasm src/bootloader/hdd2.asm -o iso/boot/hdd2.bin -f bin
        COMMAND genisoimage -input-charset utf-8 -no-emul-boot -b boot/cd.bin -o iso/melvix.iso iso
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# TEST
if (DEFINED ENV{DISPLAY})
    add_custom_command(
            TARGET kernel POST_BUILD
            COMMAND head -c 10485760 /dev/zero > iso/hdd10M.img
            COMMAND qemu-system-i386 -s -no-reboot -vga std -smp $$(nproc) -serial stdio -rtc base=localtime -m 256M -net nic,model=rtl8139,macaddr=42:42:42:42:42:42 -net user -cdrom iso/melvix.iso -drive file=iso/hdd10M.img,format=raw
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif ()

# CLEAN
add_custom_target(clean_iso COMMAND rm -rf ${CMAKE_CURRENT_SOURCE_DIR}/iso && mkdir -p ${CMAKE_CURRENT_SOURCE_DIR}/iso/boot)

# Dependencies
add_dependencies(kernel clean_iso resources user)
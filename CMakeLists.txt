cmake_minimum_required(VERSION 3.0)
project(Melvix)
enable_language(ASM)
# set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_COLOR_MAKEFILE on)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

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
set(CMAKE_C_FLAGS "-Wall -Wno-int-conversion -fstrength-reduce -fomit-frame-pointer -finline-functions -nostdlib -ffreestanding -std=gnu99 -g -ggdb -c")
set(CMAKE_EXE_LINKER_FLAGS "-ffreestanding -O2 -nostdlib -g -ggdb")
set(CMAKE_ASM_COMPILE_OBJECT "<CMAKE_ASM_COMPILER> <SOURCE> -o <OBJECT> -f elf32 -O0")
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_LINKER> ${CMAKE_EXE_LINKER_FLAGS} <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_ASM_LINK_EXECUTABLE "<CMAKE_LINKER> ${CMAKE_EXE_LINKER_FLAGS} <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

# Recursive sources
file(GLOB_RECURSE kernel_sources src/kernel/*.c src/kernel/*.asm)
file(GLOB_RECURSE resources_sources src/resources/*.c)
file(GLOB_RECURSE user_sources src/userspace/programs/*.c)
file(GLOB_RECURSE user_libc_sources src/userspace/libc/*.c)

# KERNEL
add_executable(kernel ${kernel_sources})
target_include_directories(kernel PRIVATE "src")
set_target_properties(kernel PROPERTIES OUTPUT_NAME "${CMAKE_CURRENT_SOURCE_DIR}/build/kernel/kernel.bin")
target_compile_options(kernel PRIVATE "-D ${NETWORK}")
target_link_libraries(kernel PRIVATE "-T ${CMAKE_CURRENT_SOURCE_DIR}/src/kernel/linker.ld")

# RESOURCES
add_executable(resources ${resources_sources})
set_target_properties(resources PROPERTIES OUTPUT_NAME "${CMAKE_CURRENT_SOURCE_DIR}/build/res/font.o")
target_compile_options(resources PRIVATE "-Os") # This is needed to preserve my fixed function order ig
add_custom_command(
        TARGET resources POST_BUILD
        COMMAND cross/opt/bin/i686-elf-objcopy -O binary build/res/font.o build/res/font.bin
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# USERSPACE
add_library(userspace_lib ${user_libc_sources})
target_include_directories(userspace_lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/userspace/libc/")
foreach(loop_file ${user_sources})
	get_filename_component(testname ${loop_file} NAME_WE)
	add_executable(${testname} ${loop_file})
	target_include_directories(${testname} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/userspace/libc/")
	target_link_libraries(${testname} userspace_lib "-T ${CMAKE_CURRENT_SOURCE_DIR}/src/userspace/linker.ld")
	set_target_properties(${testname} PROPERTIES OUTPUT_NAME "${CMAKE_CURRENT_SOURCE_DIR}/build/user/${testname}")
endforeach(loop_file ${user_sources})

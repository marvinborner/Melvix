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

# Compiler and linker
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS " ")
set(CMAKE_C_COMPILER "${CMAKE_CURRENT_SOURCE_DIR}/cross/opt/bin/i686-elf-gcc")
set(CMAKE_ASM_COMPILER "nasm")
set(CMAKE_LINKER "${CMAKE_CURRENT_SOURCE_DIR}/cross/opt/bin/i686-elf-gcc")
set(CMAKE_ASM_LINKER_PREFERENCE ${CMAKE_LINKER})
set(CMAKE_C_LINKER_PREFERENCE ${CMAKE_LINKER})

# Compiler and linker flags
set(CMAKE_C_FLAGS "-Wall -Wextra -nostdlib -nostdinc -ffreestanding -std=gnu99 -pedantic-errors -c -I${CMAKE_CURRENT_SOURCE_DIR}/src/lib/")
set(CMAKE_EXE_LINKER_FLAGS "-ffreestanding -O2 -nostdlib -nostdinc")
set(CMAKE_ASM_COMPILE_OBJECT "<CMAKE_ASM_COMPILER> <SOURCE> -o <OBJECT> -f bin -O0")
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_LINKER> ${CMAKE_EXE_LINKER_FLAGS} <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_ASM_LINK_EXECUTABLE "<CMAKE_LINKER> ${CMAKE_EXE_LINKER_FLAGS} <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

# Recursive sources
file(GLOB_RECURSE sources src/*.c)
file(GLOB_RECURSE apps_sources apps/*.c)

# MELVIX
add_executable(melvix ${sources})
set_target_properties(melvix PROPERTIES OUTPUT_NAME "${CMAKE_CURRENT_SOURCE_DIR}/build/melvix.bin")
target_link_libraries(melvix PRIVATE "-Ttext 0x0050000")

# APPS
foreach(loop_file ${apps_sources})
	get_filename_component(testname ${loop_file} NAME_WE)
	add_executable(${testname} ${loop_file})
	set_target_properties(${testname} PROPERTIES OUTPUT_NAME "${CMAKE_CURRENT_SOURCE_DIR}/disk/bin/${testname}")
endforeach(loop_file ${user_sources})

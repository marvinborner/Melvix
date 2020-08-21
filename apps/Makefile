# MIT License, Copyright (c) 2020 Marvin Borner

COBJS = init.o wm.o
CC = ../cross/opt/bin/i686-elf-gcc
LD = ../cross/opt/bin/i686-elf-ld
OC = ../cross/opt/bin/i686-elf-objcopy

# Flags to make the binary smaller TODO: Remove after indirect pointer support!
# TODO: Fix optimization flags (relocation of functions)
CSFLAGS = -mpreferred-stack-boundary=2 -fno-asynchronous-unwind-tables -O0

CFLAGS = $(CSFLAGS) -Wall -Wextra -nostdlib -nostdinc -fno-builtin -mgeneral-regs-only -std=c99 -m32 -pedantic-errors -I../libc/inc/ -I../libgui/inc/ -fPIE -Duserspace

all: $(COBJS)

%.o: %.c
	@mkdir -p ../build/apps/
	@$(CC) -c $(CFLAGS) $< -o $@
	@$(LD) -o $(@:.o=.elf) -Tlink.ld -L../build/ $@ -lc -lgui
	@$(OC) -O binary $(@:.o=.elf) ../build/apps/$(@:.o=)

# %.o: %.c
# 	@mkdir -p ../build/apps/
# 	@$(CC) -c $(CFLAGS) $< -o $@
# 	@$(CC) -r $(CFLAGS) -o ../build/apps/$(@:.o=) -L../build $< -lc

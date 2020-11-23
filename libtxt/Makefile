# MIT License, Copyright (c) 2020 Marvin Borner

COBJS = keymap.o xml.o
CC = ccache ../cross/opt/bin/i686-elf-gcc
LD = ccache ../cross/opt/bin/i686-elf-ld
AR = ccache ../cross/opt/bin/i686-elf-ar

WARNINGS = -Wall -Wextra -pedantic-errors -Wshadow -Wpointer-arith -Wwrite-strings -Wredundant-decls -Wnested-externs -Wno-long-long
CFLAGS = $(WARNINGS) -nostdlib -nostdinc -fno-builtin -mgeneral-regs-only -std=c99 -m32 -Iinc/ -I../libc/inc/ -fPIE -Duserspace -Ofast

all: libtxt

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@

libtxt: $(COBJS)
	@mkdir -p ../build/
	@$(AR) rcs ../build/libtxt.a $+

clean:
	@find . -name "*.o" -type f -delete

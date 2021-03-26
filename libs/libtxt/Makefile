# MIT License, Copyright (c) 2020 Marvin Borner

COBJS = keymap.o #xml.o html.o
CC = ccache ../../cross/opt/bin/i686-elf-gcc
LD = ccache ../../cross/opt/bin/i686-elf-ld
AR = ccache ../../cross/opt/bin/i686-elf-ar

CFLAGS = $(CFLAGS_DEFAULT) -I../ -I../libc/inc/ -Duserspace

all: libtxt

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@

libtxt: $(COBJS)
	@mkdir -p ../../build/
	@$(AR) rcs ../../build/libtxt.a $+

clean:
	@find . -name "*.o" -type f -delete

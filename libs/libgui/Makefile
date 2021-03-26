# MIT License, Copyright (c) 2020 Marvin Borner

COBJS = psf.o \
	bmp.o \
	png.o \
	gfx.o \
	gui.o \
	msg.o
CC = ccache ../../cross/opt/bin/i686-elf-gcc
LD = ccache ../../cross/opt/bin/i686-elf-ld
AR = ccache ../../cross/opt/bin/i686-elf-ar

CFLAGS = $(CFLAGS_DEFAULT) -I../ -I../libc/inc/ -Duserspace

all: libgui

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@

libgui: $(COBJS)
	@mkdir -p ../../build/
	@$(AR) rcs ../../build/libgui.a $+

clean:
	@find . -name "*.o" -type f -delete

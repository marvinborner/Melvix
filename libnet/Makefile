# MIT License, Copyright (c) 2020 Marvin Borner

COBJS = dns.o http.o ip.o
CC = ccache ../cross/opt/bin/i686-elf-gcc
LD = ccache ../cross/opt/bin/i686-elf-ld
AR = ccache ../cross/opt/bin/i686-elf-ar

CFLAGS = $(CFLAGS_DEFAULT) -Iinc/ -I../libc/inc/ -fPIE -Duserspace

all: libtxt

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@

libtxt: $(COBJS)
	@mkdir -p ../build/
	@$(AR) rcs ../build/libnet.a $+

clean:
	@find . -name "*.o" -type f -delete

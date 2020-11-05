# MIT License, Copyright (c) 2020 Marvin Borner

COBJS = init.o wm.o mandelbrot.o window.o exec.o files.o test.o cc.o
CC = ccache ../cross/opt/bin/i686-elf-gcc
LD = ccache ../cross/opt/bin/i686-elf-ld
OC = ccache ../cross/opt/bin/i686-elf-objcopy

WARNINGS = -Wall -Wextra -pedantic-errors -Wshadow -Wpointer-arith -Wwrite-strings -Wredundant-decls -Wnested-externs -Wno-long-long
CFLAGS = $(WARNINGS) -nostdlib -nostdinc -fno-builtin -std=c99 -m32 -I../libc/inc/ -I../libgui/inc/ -I../libtxt/inc/ -fPIE -Duserspace -Ofast

all: $(COBJS)

%.o: %.c
	@mkdir -p ../build/apps/
	@$(CC) -c $(CFLAGS) $< -o $@
	@$(LD) -o $(@:.o=.elf) -Tlink.ld -L../build/ $@ -lgui -ltxt -lc
	@$(OC) -O binary $(@:.o=.elf) ../build/apps/$(@:.o=)
# @cp $(@:.o=.elf) ../build/apps/$(@:.o=.dbg)

# %.o: %.c
# 	@mkdir -p ../build/apps/
# 	@$(CC) -c $(CFLAGS) $< -o $@
# 	@$(CC) -r $(CFLAGS) -o ../build/apps/$(@:.o=) -L../build $< -lc

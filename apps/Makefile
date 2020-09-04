# MIT License, Copyright (c) 2020 Marvin Borner

COBJS = init.o wm.o mandelbrot.o window.o test.o
CC = ../cross/opt/bin/i686-elf-gcc
LD = ../cross/opt/bin/i686-elf-ld
OC = ../cross/opt/bin/i686-elf-objcopy

CFLAGS = -Wall -Wextra -nostdlib -nostdinc -fno-builtin -std=c99 -m32 -pedantic-errors -I../libc/inc/ -I../libgui/inc/ -fPIE -Duserspace -Ofast

all: $(COBJS)

%.o: %.c
	@mkdir -p ../build/apps/
	@$(CC) -c $(CFLAGS) $< -o $@
	@$(LD) -o $(@:.o=.elf) -Tlink.ld -L../build/ $@ -lc -lgui
	@$(OC) -O binary $(@:.o=.elf) ../build/apps/$(@:.o=)
# @cp $(@:.o=.elf) ../build/apps/$(@:.o=.dbg)

# %.o: %.c
# 	@mkdir -p ../build/apps/
# 	@$(CC) -c $(CFLAGS) $< -o $@
# 	@$(CC) -r $(CFLAGS) -o ../build/apps/$(@:.o=) -L../build $< -lc

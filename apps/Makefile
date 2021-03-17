# MIT License, Copyright (c) 2020 Marvin Borner

COBJS = init.o idle.o wm.o test.o window.o #mandelbrot.o window.o exec.o files.o test.o cc.o browser.o server.o
CC = ccache ../cross/opt/bin/i686-elf-gcc
LD = ccache ../cross/opt/bin/i686-elf-ld
OC = ccache ../cross/opt/bin/i686-elf-objcopy

CFLAGS = $(CFLAGS_DEFAULT) -I../libc/inc/ -I../libgui/inc/ -I../libtxt/inc/ -Duserspace

all: $(COBJS)

%.o: %.c
	@mkdir -p ../build/apps/
	@$(CC) -c $(CFLAGS) $< -o $@
	@$(LD) -o $(@:.o=.elf) -L../build/ $@ -lgui -ltxt -lc
	@cp $(@:.o=.elf) ../build/apps/$(@:.o=)

# %.o: %.c
# 	@mkdir -p ../build/apps/
# 	@$(CC) -c $(CFLAGS) $< -o $@
# 	@$(CC) -r $(CFLAGS) -o ../build/apps/$(@:.o=) -L../build $< -lc

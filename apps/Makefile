# MIT License, Copyright (c) 2020 Marvin Borner

COBJS = init.o wm.o test.o window.o #mandelbrot.o window.o exec.o files.o test.o cc.o browser.o server.o
CC = ccache ../cross/opt/bin/i686-elf-gcc
LD = ccache ../cross/opt/bin/i686-elf-ld
OC = ccache ../cross/opt/bin/i686-elf-objcopy

CFLAGS = $(CFLAGS_DEFAULT) -I../libc/inc/ -I../libgui/inc/ -I../libtxt/inc/ -I../libnet/inc/ -fPIE -Duserspace

all: $(COBJS)

%.o: %.c
	@mkdir -p ../build/apps/
	@$(CC) -c $(CFLAGS) $< -o $@
	@$(LD) -o $(@:.o=.elf) -Tlink.ld -L../build/ $@ -lgui -ltxt -lnet -lc
	@$(OC) -O binary $(@:.o=.elf) ../build/apps/$(@:.o=)
	@cp $(@:.o=.elf) ../build/apps/$(@:.o=.elf)

# %.o: %.c
# 	@mkdir -p ../build/apps/
# 	@$(CC) -c $(CFLAGS) $< -o $@
# 	@$(CC) -r $(CFLAGS) -o ../build/apps/$(@:.o=) -L../build $< -lc

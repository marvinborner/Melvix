# MIT License, Copyright (c) 2020 Marvin Borner

COBJS = psf.o \
	png.o \
	gfx.o \
	gui.o \
	msg.o \
	widgets.o

CFLAGS = $(CFLAGS_DEFAULT) -I$(LIBS) -I$(LIBS)/libc/inc/ -pie -fPIE -fPIC -DUSER

all: libgui

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@

libgui: $(COBJS)
	@mkdir -p $(BUILD)
	@$(AR) rcs $(BUILD)/libgui.a $+

clean:
	@find . -name "*.o" -type f -delete

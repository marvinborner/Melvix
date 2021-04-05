# MIT License, Copyright (c) 2020 Marvin Borner

COBJS = keymap.o #xml.o html.o

CFLAGS = $(CFLAGS_DEFAULT) -I$(LIBS) -I$(LIBS)/libc/inc/ -pie -fPIE -fPIC -DUSER

all: libtxt

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@

libtxt: $(COBJS)
	@mkdir -p $(BUILD)
	@$(AR) rcs $(BUILD)/libtxt.a $+

clean:
	@find . -name "*.o" -type f -delete

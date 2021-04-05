# MIT License, Copyright (c) 2021 Marvin Borner

OBJS = wm.o

all: $(OBJS)
	@$(LD) -o $(BUILD)/apps/wm $(LDFLAGS) $< -lgui -ltxt -lc

clean:
	@$(RM) -f $(OBJS)

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@

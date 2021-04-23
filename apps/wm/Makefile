# MIT License, Copyright (c) 2021 Marvin Borner

OBJS = wm.o

all: $(OBJS)
	@mkdir -p $(BUILD)/apps/wm/
	@$(LD) -o $(BUILD)/apps/wm/exec $(LDFLAGS) $^ -lgui -ltxt -lc

clean:
	@$(RM) -f $(OBJS)

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@

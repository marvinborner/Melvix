# MIT License, Copyright (c) 2021 Marvin Borner

OBJS = idle.o

all: $(OBJS)
	@$(LD) -o $(BUILD)/apps/idle $(LDFLAGS) $< -lc

clean:
	@$(RM) -f $(OBJS)

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@

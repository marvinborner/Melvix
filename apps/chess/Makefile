# MIT License, Copyright (c) 2021 Marvin Borner

OBJS = chess.o

all: $(OBJS)
	@$(LD) -o $(BUILD)/apps/chess $(LDFLAGS) $< -lgui -ltxt -lc

clean:
	@$(RM) -f $(OBJS)

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@

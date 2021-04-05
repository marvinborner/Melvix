# MIT License, Copyright (c) 2021 Marvin Borner

OBJS = init.o

all: $(OBJS)
	@$(LD) -o $(BUILD)/apps/init $(LDFLAGS) $< -lc

clean:
	@$(RM) -f $(OBJS)

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@

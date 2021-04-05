# MIT License, Copyright (c) 2021 Marvin Borner

OBJS = test.o

all: $(OBJS)
	@$(LD) -o $(BUILD)/apps/test $(LDFLAGS) $< -lc

clean:
	@$(RM) -f $(OBJS)

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@

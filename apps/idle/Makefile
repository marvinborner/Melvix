# MIT License, Copyright (c) 2021 Marvin Borner

OBJS = idle.o

all: $(OBJS)
	@mkdir -p $(BUILD)/apps/idle/
	@$(LD) -o $(BUILD)/apps/idle/exec $(LDFLAGS) $< -lc

clean:
	@$(RM) -f $(OBJS)

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@

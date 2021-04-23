# MIT License, Copyright (c) 2021 Marvin Borner

OBJS = test.o

all: $(OBJS)
	@mkdir -p $(BUILD)/apps/test/
	@$(LD) -o $(BUILD)/apps/test/exec $(LDFLAGS) $^ -lc

clean:
	@$(RM) -f $(OBJS)

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@

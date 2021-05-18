# MIT License, Copyright (c) 2021 Marvin Borner

OBJS = chess.o

all: $(OBJS)
	@mkdir -p $(BUILD)/apps/chess/
	@$(LD) -o $(BUILD)/apps/chess/exec $(LDFLAGS) $^ -lgui -ltxt -lc
ifeq ($(CONFIG_STRIP), true)
	@$(ST) --strip-all $(BUILD)/apps/chess/exec
endif

clean:
	@$(RM) -f $(OBJS)

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@

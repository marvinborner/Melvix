# MIT License, Copyright (c) 2021 Marvin Borner

OBJS = wm.o

all: $(OBJS)
	@mkdir -p $(BUILD)/apps/wm/
	@$(LD) -o $(BUILD)/apps/wm/exec $(LDFLAGS) $^ -lgui -ltxt -lc
ifeq ($(CONFIG_STRIP), true)
	@$(ST) --strip-all $(BUILD)/apps/wm/exec
endif

clean:
	@$(RM) -f $(OBJS)

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@

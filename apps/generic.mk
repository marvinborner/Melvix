# MIT License, Copyright (c) 2021 Marvin Borner

NAME ?= unknown
LIBS ?= -lc

all: $(OBJS)
	@mkdir -p $(BUILD)/apps/$(NAME)/
	@$(LD) -o $(BUILD)/apps/$(NAME)/exec $(LDFLAGS) $^ $(LIBS)
ifeq ($(CONFIG_STRIP), true)
	@$(ST) --strip-all $(BUILD)/apps/wm/exec
endif

clean:
	@$(RM) -f $(OBJS)

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@

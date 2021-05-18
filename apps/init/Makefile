# MIT License, Copyright (c) 2021 Marvin Borner

OBJS = init.o

all: $(OBJS)
	@mkdir -p $(BUILD)/apps/init/
	@$(LD) -o $(BUILD)/apps/init/exec $(LDFLAGS) $^ -lc
ifeq ($(CONFIG_STRIP), true)
	@$(ST) --strip-all $(BUILD)/apps/init/exec
endif

clean:
	@$(RM) -f $(OBJS)

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@

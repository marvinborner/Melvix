# MIT License, Copyright (c) 2021 Marvin Borner

OBJS = test.o fuzz.o

all: $(OBJS)
	@mkdir -p $(BUILD)/apps/test/
	@$(LD) -o $(BUILD)/apps/test/exec $(LDFLAGS) $^ -lc
ifeq ($(CONFIG_STRIP), true)
	@$(ST) --strip-all $(BUILD)/apps/test/exec
endif

clean:
	@$(RM) -f $(OBJS)

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@

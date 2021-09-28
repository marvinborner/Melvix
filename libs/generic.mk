# MIT License, Copyright (c) 2021 Marvin Borner

NAME ?= $(shell basename $(CURDIR))
SRC ?= $(shell pwd)/src
INC ?= $(shell pwd)/inc

CFLAGS += -I$(INC)

CSRCS = $(shell find $(SRC) -type f -name "*.c")
COBJS = $(patsubst $(SRC)/%.c,$(BUILD)/libs/$(NAME)/%_c.o,$(CSRCS))

ASRCS = $(shell find $(SRC) -type f -name "*.asm")
AOBJS = $(patsubst $(SRC)/%.asm,$(BUILD)/libs/$(NAME)/%_asm.o,$(ASRCS))

all: $(BUILD)/$(NAME).a

$(COBJS): $(BUILD)/libs/$(NAME)/%_c.o : $(SRC)/%.c
	@echo "[CC] $(NAME): $@"
	@mkdir -p $(shell dirname $@)
	@$(CC) -c $(CFLAGS) $< -o $@

$(AOBJS): $(BUILD)/libs/$(NAME)/%_asm.o : $(SRC)/%.asm
	@echo "[AS] $(NAME): $@"
	@mkdir -p $(shell dirname $@)
	@$(AS) $(ASFLAGS) $< -o $@

$(BUILD)/$(NAME).a: $(COBJS) $(AOBJS)
	@echo "[AR] $(NAME): $@"
	@$(AR) rcs $@ $+

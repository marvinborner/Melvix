# MIT License, Copyright (c) 2021 Marvin Borner

NAME ?= $(shell basename $(CURDIR))
SRC ?= ./src
INC ?= ./inc
ARCH_DIR ?= $(SRC)/arch

CFLAGS += -I$(INC) -I$(ARCH_DIR)/$(ARCH)/$(INC) -Wno-pedantic

# TODO: Make arch dir usage optional
CSRCS = $(shell find $(SRC) -path $(ARCH_DIR) -prune -false -o -type f -name "*.c")
CSRCS += $(shell find $(ARCH_DIR)/$(ARCH) -type f -name "*.c")
COBJS = $(patsubst $(SRC)/%.c,$(BUILD)/$(NAME)/%_c.o,$(CSRCS))

ASRCS = $(shell find $(SRC) -path $(ARCH_DIR) -prune -false -o -type f -name "*.asm")
ASRCS += $(shell find $(ARCH_DIR)/$(ARCH) -type f -name "*.asm")
AOBJS = $(patsubst $(SRC)/%.asm,$(BUILD)/$(NAME)/%_asm.o,$(ASRCS))

all: dir $(BUILD)/$(NAME).a

dir:
	@mkdir -p $(BUILD)/$(NAME)

$(COBJS): $(BUILD)/$(NAME)/%_c.o : $(SRC)/%.c
	@echo "[CC] $(NAME): $<"
	@$(CC) -c $(CFLAGS) $< -o $@

$(AOBJS): $(BUILD)/$(NAME)/%_asm.o : $(SRC)/%.asm
	@echo "[AS] $(NAME): $<"
	@$(AS) $(ASFLAGS) $< -o $@

$(BUILD)/$(NAME).a: $(COBJS) $(AOBJS)
	@echo "[AR] $(NAME): $(NAME).a"
	@$(AR) rcs $@ $<

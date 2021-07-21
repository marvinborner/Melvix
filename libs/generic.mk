# MIT License, Copyright (c) 2021 Marvin Borner

NAME ?= $(shell basename $(CURDIR))
SRC ?= $(shell pwd)/src
INC ?= $(shell pwd)/inc
ARCH_DIR ?= arch

CFLAGS += -I$(INC) -I$(INC)/$(ARCH_DIR)/$(ARCH) -I$(INC)/$(ARCH_DIR)/$(ARCH_MAJOR)

# TODO: Make arch dir usage optional
CSRCS = $(shell find $(SRC) -path $(SRC)/$(ARCH_DIR) -prune -false -o -type f -name "*.c")
CSRCS += $(shell find $(SRC)/$(ARCH_DIR)/$(ARCH) -type f -name "*.c")
CSRCS += $(shell find $(SRC)/$(ARCH_DIR)/$(ARCH_MAJOR) -maxdepth 1 -type f -name "*.c")
COBJS = $(patsubst $(SRC)/%.c,$(BUILD)/libs/$(NAME)/%_c.o,$(CSRCS))

ASRCS = $(shell find $(SRC) -path $(SRC)/$(ARCH_DIR) -prune -false -o -type f -name "*.asm")
ASRCS += $(shell find $(SRC)/$(ARCH_DIR)/$(ARCH) -type f -name "*.asm")
CSRCS += $(shell find $(SRC)/$(ARCH_DIR)/$(ARCH_MAJOR) -maxdepth 1 -type f -name "*.asm")
AOBJS = $(patsubst $(SRC)/%.asm,$(BUILD)/libs/$(NAME)/%_asm.o,$(ASRCS))

all: dir $(BUILD)/$(NAME).a

dir:
	@mkdir -p $(BUILD)/libs/$(NAME)

$(COBJS): $(BUILD)/libs/$(NAME)/%_c.o : $(SRC)/%.c
	@echo "[CC] $(NAME): $@"
	@$(CC) -c $(CFLAGS) $< -o $@

$(AOBJS): $(BUILD)/libs/$(NAME)/%_asm.o : $(SRC)/%.asm
	@echo "[AS] $(NAME): $@"
	@$(AS) $(ASFLAGS) $< -o $@

$(BUILD)/$(NAME).a: $(COBJS) $(AOBJS)
	@echo "[AR] $(NAME): $@"
	@$(AR) rcs $@ $+

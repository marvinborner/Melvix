# MIT License, Copyright (c) 2021 Marvin Borner

NAME = kernel
SRC = ./src
INC = ./inc
ARCH_DIR = $(SRC)/arch

CFLAGS += -I$(INC) -I$(ARCH_DIR)/$(ARCH)/$(INC) -I$(ARCH_DIR)/$(ARCH_MAJOR)/$(INC) -I$(LIBS)/libc/inc -I$(LIBS)/libc/src/arch/$(ARCH)/inc -ffreestanding -mgeneral-regs-only -Wl,-earch_init

CSRCS = $(shell find $(SRC) -path $(ARCH_DIR) -prune -false -o -type f -name "*.c")
CSRCS += $(shell find $(ARCH_DIR)/$(ARCH) -type f -name "*.c")
CSRCS += $(shell find $(ARCH_DIR)/$(ARCH_MAJOR) -maxdepth 1 -type f -name "*.c")
COBJS = $(patsubst $(SRC)/%.c,$(BUILD)/$(NAME)/%_c.o,$(CSRCS))

ASRCS = $(shell find $(SRC) -path $(ARCH_DIR) -prune -false -o -type f -name "*.asm")
ASRCS += $(shell find $(ARCH_DIR)/$(ARCH) -type f -name "*.asm")
CSRCS += $(shell find $(ARCH_DIR)/$(ARCH_MAJOR) -maxdepth 1 -type f -name "*.asm")
AOBJS = $(patsubst $(SRC)/%.asm,$(BUILD)/$(NAME)/%_asm.o,$(ASRCS))

all: $(BUILD)/$(NAME).elf

$(COBJS): $(BUILD)/$(NAME)/%_c.o : $(SRC)/%.c
	@echo "[CC] $(NAME): $<"
	@mkdir -p $(shell dirname $@)
	@$(CC) -c $(CFLAGS) $< -o $@

$(AOBJS): $(BUILD)/$(NAME)/%_asm.o : $(SRC)/%.asm
	@echo "[AS] $(NAME): $<"
	@mkdir -p $(shell dirname $@)
	@$(AS) $(ASFLAGS) $< -o $@

$(BUILD)/$(NAME).elf: $(COBJS) $(AOBJS)
	@echo "[LD] $(NAME): $(NAME).elf"
	@$(LD) -N -z max-page-size=0x1000 -eboot_entry -Tlink.ld -o $@ -L$(BUILD) $+ -lc

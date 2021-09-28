# MIT License, Copyright (c) 2021 Marvin Borner

NAME = kernel
SRC = $(shell pwd)/src
INC = $(shell pwd)/inc
ARCH_DIR = arch

CFLAGS += -I$(INC) -I$(LIBS)/libc/inc -Wl,-eboot_entry

CSRCS = $(shell find $(SRC) -type f -name "*.c")
COBJS = $(patsubst $(SRC)/%.c,$(BUILD)/$(NAME)/%_c.o,$(CSRCS))

ASRCS = $(shell find $(SRC) -type f -name "*.asm")
AOBJS = $(patsubst $(SRC)/%.asm,$(BUILD)/$(NAME)/%_asm.o,$(ASRCS))

all: $(BUILD)/$(NAME).elf

$(COBJS): $(BUILD)/$(NAME)/%_c.o : $(SRC)/%.c
	@echo "[CC] $(NAME): $@"
	@mkdir -p $(shell dirname $@)
	@$(CC) -c $(CFLAGS) $< -o $@

$(AOBJS): $(BUILD)/$(NAME)/%_asm.o : $(SRC)/%.asm
	@echo "[AS] $(NAME): $@"
	@mkdir -p $(shell dirname $@)
	@$(AS) $(ASFLAGS) $< -o $@

$(BUILD)/$(NAME).elf: $(COBJS) $(AOBJS)
	@echo "[LD] $(NAME): $@"
	@$(LD) -N -z max-page-size=0x1000 -eboot_entry -Tlink.ld -o $@ -L$(BUILD) $+ -lc

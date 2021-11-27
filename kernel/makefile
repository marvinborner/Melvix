# Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
# SPDX-License-Identifier: MIT

NAME = kernel
SRC = ./src
INC = ./inc

CFLAGS += -I$(INC) -I$(LIBS)/libc/inc

SRCS = $(shell find $(SRC) -path $(SRC)/drivers -prune -false -o -type f -name '*.c' -or -name '*.asm')
SRCS += $(SRC)/drivers/install.c $(DRIVER_INTERRUPT) $(DRIVER_KEYBOARD) $(DRIVER_MOUSE) $(DRIVER_NETWORK) $(DRIVER_SERIAL) $(DRIVER_VIDEO)
OBJS = $(SRCS:%=$(BUILD)/$(NAME)/%.o)

$(BUILD)/$(NAME).elf: $(OBJS)
	@echo "[LD] $(NAME): $@"
	@$(LD) -N -z max-page-size=0x1000 -eboot_entry -Tlink.ld -o $@ -L$(BUILD) $+ -lc

$(BUILD)/$(NAME)/%.c.o: %.c
	@echo "[CC] $(NAME): $@"
	@mkdir -p $(shell dirname $@)
	@$(CC) -c $(CFLAGS) $< -o $@

$(BUILD)/$(NAME)/%.asm.o: %.asm
	@echo "[AS] $(NAME): $@"
	@mkdir -p $(shell dirname $@)
	@$(AS) $(ASFLAGS) $< -o $@

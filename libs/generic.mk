# Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
# SPDX-License-Identifier: MIT

NAME ?= $(shell basename $(CURDIR))
SRC = ./src
INC = ./inc

CFLAGS += -I$(INC)

SRCS = $(shell find $(SRC) -type f -name '*.c' -or -name '*.asm')
OBJS = $(SRCS:%=$(BUILD)/libs/$(NAME)/%.o)

$(BUILD)/$(NAME).a: $(OBJS)
	@echo "[AR] $(NAME): $@"
	@$(AR) rcs $@ $+

$(BUILD)/libs/$(NAME)/%.c.o: %.c
	@echo "[CC] $(NAME): $@"
	@mkdir -p $(shell dirname $@)
	@$(CC) -c $(CFLAGS) $< -o $@

$(BUILD)/libs/$(NAME)/%.asm.o: %.asm
	@echo "[AS] $(NAME): $@"
	@mkdir -p $(shell dirname $@)
	@$(AS) $(ASFLAGS) $< -o $@

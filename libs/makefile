# Copyright (c) 2021, Marvin Borner <melvix@marvinborner.de>
# SPDX-License-Identifier: MIT

DIRS = $(wildcard */.)

all: $(DIRS)
$(DIRS):
	@$(MAKE) --no-print-directory -C $@

.PHONY: all $(DIRS)

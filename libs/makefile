# MIT License, Copyright (c) 2021 Marvin Borner

DIRS = $(wildcard */.)

all: $(DIRS)
$(DIRS):
	@$(MAKE) --no-print-directory -C $@

.PHONY: all $(DIRS)

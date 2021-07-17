# MIT License, Copyright (c) 2021 Marvin Borner

LIBS = $(wildcard */.)

all: $(LIBS)
$(LIBS):
	@$(MAKE) --no-print-directory -C $@

.PHONY: all $(LIBS)

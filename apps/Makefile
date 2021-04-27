# MIT License, Copyright (c) 2020 Marvin Borner

CFLAGS = $(CFLAGS_DEFAULT) -I$(LIBS)/ -I$(LIBS)/libc/inc/ -pie -fPIE -fPIC -DUSER
LDFLAGS = --section-start=.text=0x42000000 -L$(BUILD)
DIRS = $(wildcard */.)

ifneq ($(DEBUG), 1)
	LDFLAGS += -pie -no-dynamic-linker
endif

export

all: $(DIRS)
$(DIRS):
	@mkdir -p $(BUILD)/apps/
	@$(MAKE) --no-print-directory -C $@
	@$(MAKE) clean --no-print-directory -C $@

.PHONY: all $(DIRS)

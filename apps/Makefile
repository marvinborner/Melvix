# MIT License, Copyright (c) 2020 Marvin Borner

CFLAGS = $(CFLAGS_DEFAULT) -I$(LIBS)/ -I$(LIBS)/libc/inc/ -DUSER
LDFLAGS = --section-start=.text=0x42000000 -L$(BUILD)
DIRS = $(wildcard */.)

ifeq ($(CONFIG_USER_PIE), true)
	LDFLAGS += -pie -no-dynamic-linker
	CFLAGS += -pie -fPIE -fPIC
endif

export

all: $(DIRS)
$(DIRS):
	@mkdir -p $(BUILD)/apps/
	@$(MAKE) --no-print-directory -C $@
	@$(MAKE) clean --no-print-directory -C $@

.PHONY: all $(DIRS)

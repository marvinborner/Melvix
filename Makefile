# MIT License, Copyright (c) 2020 Marvin Borner

all: compile clean

compile:
	@$(MAKE) clean --no-print-directory -C libc/
	@$(MAKE) libc --no-print-directory -C libc/
	@echo "Compiled libc"
	@$(MAKE) clean --no-print-directory -C libc/
	@$(MAKE) libk --no-print-directory -C libc/
	@echo "Compiled libk"
	@$(MAKE) --no-print-directory -C libgui/
	@echo "Compiled libgui"
	@$(MAKE) --no-print-directory -C libtxt/
	@echo "Compiled libtxt"
	@$(MAKE) --no-print-directory -C kernel/
	@echo "Compiled kernel"
	@$(MAKE) --no-print-directory -C boot/
	@echo "Compiled boot"
	@$(MAKE) --no-print-directory -C apps/
	@echo "Compiled apps"

clean:
	@find kernel/ apps/ libc/ libtxt/ libgui/ boot/ \( -name "*.o" -or -name "*.a" -or -name "*.elf" -or -name "*.bin" \) -type f -delete

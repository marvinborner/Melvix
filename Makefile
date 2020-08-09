# MIT License, Copyright (c) 2020 Marvin Borner

all: compile clean

compile:
	@$(MAKE) clean --no-print-directory -C lib/
	@$(MAKE) libc --no-print-directory -C lib/
	@echo "Compiled libc"
	@$(MAKE) clean --no-print-directory -C lib/
	@$(MAKE) libk --no-print-directory -C lib/
	@echo "Compiled libk"
	@$(MAKE) --no-print-directory -C kernel/
	@echo "Compiled kernel"
	@$(MAKE) --no-print-directory -C apps/
	@echo "Compiled apps"

clean:
	@find kernel/ apps/ lib/ \( -name "*.o" -or -name "*.a" -or -name "*.elf" -or -name "*.bin" \) -type f -delete

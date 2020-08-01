# MIT License, Copyright (c) 2020 Marvin Borner

all: compile clean

compile:
	@$(MAKE) --no-print-directory -C src/
	@$(MAKE) --no-print-directory -C apps/

clean:
	@find src/ apps/ \( -name "*.o" -or -name "*.elf" \) -type f -delete

# MIT License, Copyright (c) 2020 Marvin Borner

CFLAGS_OPTIMIZATION = -finline -finline-functions -Ofast
CFLAGS_WARNINGS = -Wall -Wextra -Werror -Wshadow -Wpointer-arith -Wwrite-strings -Wredundant-decls -Wnested-externs -Wformat=2 -Wmissing-declarations -Wstrict-prototypes -Wmissing-prototypes -Wcast-qual -Wswitch-default -Wswitch-enum -Wlogical-op -Wunreachable-code -Wundef -Wold-style-definition -Wvla -pedantic-errors
CFLAGS_DEFAULT = $(CFLAGS_WARNINGS) $(CFLAGS_OPTIMIZATION) -std=c99 -m32 -nostdlib -nostdinc -fno-builtin -fno-profile-generate -fno-omit-frame-pointer -fno-common -fno-asynchronous-unwind-tables -mno-red-zone -mno-80387 -mno-mmx -mno-sse -mno-sse2 # Everything after -fno-builtin is optional

all: compile

# TODO: Fix stack protector in userspace
# TODO: Fix ubsan in userspace - Os should be a temporary fix
debug: CFLAGS_DEFAULT += -Wno-error -ggdb3 -s -Os -fsanitize=undefined -fstack-protector-all
debug: compile

export

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
	@find kernel/ apps/ libc/ libtxt/ libgui/ libnet/ boot/ \( -name "*.o" -or -name "*.a" -or -name "*.elf" -or -name "*.bin" \) -type f -delete

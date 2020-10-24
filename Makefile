# MIT License, Copyright (c) 2020 Marvin Borner

# Kernel optimization
OPTIMIZATION = -Ofast

# Remove tree optimizations for kernel - TODO: Why?
CFLAGS_EXTRA = -fno-tree-bit-ccp -fno-tree-builtin-call-dce -fno-tree-ccp -fno-tree-ch -fno-tree-coalesce-vars -fno-tree-copy-prop -fno-tree-dce -fno-tree-dominator-opts -fno-tree-dse -fno-tree-fre -fno-tree-pta -fno-tree-sink -fno-tree-slsr -fno-tree-sra -fno-tree-ter

export

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

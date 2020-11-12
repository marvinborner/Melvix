# MIT License, Copyright (c) 2020 Marvin Borner

# Kernel optimization
OPTIMIZATION = -Ofast

# Remove tree optimizations for kernel
#CFLAGS_EXTRA = -fno-tree-bit-ccp -fno-tree-builtin-call-dce -fno-tree-ccp -fno-tree-ch -fno-tree-coalesce-vars -fno-tree-copy-prop -fno-tree-dce -fno-tree-dominator-opts -fno-tree-dse -fno-tree-fre -fno-tree-pta -fno-tree-sink -fno-tree-slsr -fno-tree-sra -fno-tree-ter -fno-tree-loop-vectorize -fno-inline-functions -fno-inline-functions-called-once
# Remove ipa optimizations for kernel
#CFLAGS_EXTRA += -fno-inline-functions -fno-inline-functions-called-once -fno-reorder-functions -fno-reorder-blocks -fno-reorder-blocks-and-partition -fno-ipa-profile -fno-ipa-pure-const -fno-ipa-reference -fno-ipa-reference-addressable -fno-merge-constants -fno-ipa-bit-cp -fno-ipa-cp -fno-ipa-icf -fno-ipa-ra -fno-ipa-sra -fno-ipa-vrp -fno-ipa-cp-clone
#CFLAGS_EXTRA = -pie -fPIE -mno-80387 -mno-mmx -mno-sse -mno-sse2 -fno-asynchronous-unwind-tables

all: compile

debug: DEBUG = -ggdb3 -s
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
	@find kernel/ apps/ libc/ libtxt/ libgui/ boot/ \( -name "*.o" -or -name "*.a" -or -name "*.elf" -or -name "*.bin" \) -type f -delete

# MIT License, Copyright (c) 2020 Marvin Borner

# TODO: Remove cpu from libc?
COBJS = crt/crt0.o \
	sanitize.o \
	errno.o \
	str.o \
	alloc.o \
	mem.o \
	math.o \
	crypto.o \
	conv.o \
	print.o \
	sys.o \
	list.o \
	stack.o \
	random.o

CFLAGS = $(CFLAGS_DEFAULT) -Iinc/
ASFLAGS = -f elf32

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@

libc: CFLAGS += -pie -fPIE -fPIC -DUSER
libc: $(COBJS)
	@mkdir -p $(BUILD)
	@$(AS) $(ASFLAGS) crt/crti.asm -o $(BUILD)/crti.o
	@$(AS) $(ASFLAGS) crt/crtn.asm -o $(BUILD)/crtn.o
	@$(AR) rcs $(BUILD)/libc.a $(BUILD)/crti.o $(BUILD)/crti.o $+

libk: CFLAGS += -DKERNEL -ffreestanding -I$(KERNEL)/inc/ $(CFLAGS_EXTRA)
libk: $(COBJS)
	@mkdir -p $(BUILD)
	@$(AR) rcs $(BUILD)/libk.a $+

clean:
	@find . -name "*.o" -type f -delete

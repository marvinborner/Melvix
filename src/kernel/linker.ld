ENTRY(boot)

phys = 0x100000;
offset = 0xC0000000;
virt = offset + phys;

SECTIONS
{
	. = virt;

	.text : AT (ADDR (.text) - offset)
	{
		*(.text)
		*(.rodata)
		. = ALIGN(0x1000);
	}

	.data : AT (ADDR (.data) - offset)
	{
		*(.data)
		. = ALIGN(0x1000);
	}

	.bss : AT (ADDR (.bss) - offset)
	{
		*(.COMMON)
		*(.bss)
		. = ALIGN(0x1000);
	}

	KERNEL_END = .;
}
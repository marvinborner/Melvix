OUTPUT_FORMAT("binary")
OUTPUT_ARCH(i386)
ENTRY(kernel_main)
phys = 0x00050000;

SECTIONS
{
	kernel_start = .;

	.text phys : AT(phys) {
		code = .;
		*(.text)
		*(.rodata)
		. = ALIGN(4096);
	}

	.data : AT(phys + (data - code))
	{
		data = .;
		*(.data)
		. = ALIGN(4096);
	}

	.bss : AT(phys + (bss - code))
	{
		bss = .;
		*(.bss)
		. = ALIGN(4096);
	}

	kernel_end = .;
}

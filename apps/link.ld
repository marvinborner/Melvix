OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS
{
	. = 0x40000000;

	.text : {
		code = .;
		*(.text)
		*(.rodata)
		. = ALIGN(4096);
	}

	.data : {
		data = .;
		*(.data)
		. = ALIGN(4096);
	}

	.bss : {
		bss = .;
		*(.bss)
		. = ALIGN(4096);
	}

	. = ALIGN(4096);
}

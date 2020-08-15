OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(main)

SECTIONS
{
	. = 0x00000000;

	.text : {
		*(.text.main)
		*(.text*)
	}

	.rodata : {
		*(.rodata)
	}

	. = ALIGN(4096);

	.data : {
		*(.data)
	}

	. = ALIGN(4096);

	.bss : {
		*(.bss)
	}

	. = ALIGN(4096);

	_GLOBAL_OFFSET_TABLE_ = .;

	. = ALIGN(4096);
}

ENTRY(_start)

SECTIONS {
	.text : {
		*(.text)
		. = ALIGN(4096);
	}

	.data :	{
		*(.data)
		*(.rodata)
		. = ALIGN(4096);
	}

	.bss : {
		*(.bss)
		. = ALIGN(4096);
	}
}
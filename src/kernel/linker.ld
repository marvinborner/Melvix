ENTRY(_start)

SECTIONS {
	. = 1M;

	.text BLOCK(4K) : ALIGN(4K) {
		*(.start_section)
		*(.text)
	}

	.rodata BLOCK(4K) : ALIGN(4K) {
		*(.rodata)
	}

	.data BLOCK(4K) : ALIGN(4K) {
		*(.data)
	}

	.bss BLOCK(4K) : ALIGN(4K) {
		*(COMMON)
		*(.bss)
	}

	.end_section BLOCK(4K) : ALIGN(4K) {
		*(.end_section)
	}
}

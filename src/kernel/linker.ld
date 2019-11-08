ENTRY(_start)

/* Where the sections of the object files will be put in the final image */
SECTIONS {
	/* Begin @ 1 MB */
	. = 1M;

	/* Put the multiboot header. Next, the .text section. */
	.text BLOCK(4K) : ALIGN(4K) {
		*(.ezlocation)
		*(.text)
	}

	/* Read-only data. */
	.rodata BLOCK(4K) : ALIGN(4K) {
		*(.rodata)
	}

	/* Read-write data (initialized) */
	.data BLOCK(4K) : ALIGN(4K) {
		*(.data)
	}

	/* Read-write data (uninitialized) and stack */
	.bss BLOCK(4K) : ALIGN(4K) {
		*(COMMON)
		*(.bss)
	}

	.sizedetect BLOCK(4K) : ALIGN(4K) {
		*(.sizedetect)
	}
}

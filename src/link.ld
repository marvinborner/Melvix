ENTRY(boot)

SECTIONS
{
	. = 1M;

	.text BLOCK(4K) : ALIGN(4K)
	{
		text_start = .;
		*(.text)
		*(.rodata)
	}

	.data BLOCK(4K) : ALIGN(4K)
	{
		data_start = .;
		*(.data)
	}

	.bss BLOCK(4K) : ALIGN(4K)
	{
		bss_start = .;
		*(.COMMON)
		*(.bss)
	}

	kernel_end = .;
}

/* MIT License, Copyright (c) 2021 Marvin Borner */

OUTPUT_ARCH(i386)
ENTRY(boot_entry)
phys = 0x00100000;

SECTIONS
{
	. = phys;

	kernel_ro_start = .;

	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.text*)
	}

	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata*)
	}

	kernel_ro_end = .;

	kernel_rw_start = .;

	/* Clear after init */
	.temp_clear BLOCK(4K) : ALIGN(4K)
	{
		kernel_temp_clear_start = .;
		*(.temp_clear)
		kernel_temp_clear_end = .;
		. = ALIGN(4K);
	}

	/* Make read-only after init */
	.temp_protect BLOCK(4K) : ALIGN(4K)
	{
		kernel_temp_protect_start = .;
		*(.temp_protect)
		kernel_temp_protect_end = .;
		. = ALIGN(4K);
	}

	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data*)
	}

	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}

	kernel_rw_end = .;
}

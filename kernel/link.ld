OUTPUT_FORMAT("binary")
OUTPUT_ARCH(i386)
ENTRY(kernel_main)
phys = 0x00009000;

SECTIONS
{
	. = phys;

	kernel_ro_start = .;
	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.text)
	}

	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
	}
	kernel_ro_end = .;


	kernel_rw_start = .;
	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}

	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}
	kernel_rw_end = .;
}

# Bootloader

Hello there!

This directory is the home of my self-written bootloader. It supports ATA disk detection, EXT2 loading, basic ELF parsing/mapping and much more! You can find more information about the general memory map in `docs/MMAP.md`.

## Execution sequence

The bootloader starts in the first stage at `0x7c00` - which is the location of the MBR - by clearing the screen, finding the disk, checking for basic addressing support and finally loading the second stage. The first stage has a max size of 512 bytes, thanks to some ancient BIOS standards. That's why not much is happening in here.

The second stage searches and loads the third stage (`load.c`) using a very minimal ext2 implementation (still in assembly!). The second stage also loads the memory map and the VESA video map into memory and sets the highest available video mode. After the A20 line has been enabled and the GDT and TSS have been set, it's time to jump to the protected mode. After the computer arrived in protected mode, the third stage gets executed, with the memory and VESA map as its parameters. Finally.

The third stage (`load.c`) installs a basic serial connection for logging purposes and probes ATA disk drives. After a suitable disk has been found, the third stage will load the kernel binary into memory using an EXT2 driver and basic ELF parsing/mapping. Ultimately, the bootloader will jump to the kernel. The rest is history.

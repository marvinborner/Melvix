# Memory map

These tables should be irrelevant for most users/developers. You may find these interesting for finding bugs or potential exploits, though.

**Note**: All memory endings in the tables are exclusive.

## Available (QEMU with 256M-2G)

|    Start     |     End      | Available |
| :----------: | :----------: | :-------: |
| `0x00000000` | `0x0009fc00` |     Y     |
| `0x0009fc00` | `0x000a0000` |     N     |
| `0x000f0000` | `0x00100000` |     N     |
| `0x00100000` | `0x????????` |     Y     |
| `0x????????` | `0x40000000` |     N     |
| `0x40000000` | `0xffffffff` |     ?     |

## Physical/virtual memory map (theoretical max values, mostly way smaller in practice)

|    Start     |     End      |           Name            |
| :----------: | :----------: | :-----------------------: |
| `0x00000000` | `0x00000500` |         Unusable          |
| `0x00000500` | `0x00000600` |        Memory map         |
| `0x00007c00` | `0x00007e00` | Bootloader (first stage)  |
| `0x00007e00` | `0x00008000` | Bootloader (second stage) |
| `0x00009000` | `0x00009???` | Bootloader (third stage)  |
| `0x0000c000` | `0x0008????` |      Bootloader heap      |
| `0x00100000` | `0x001?????` |          Kernel           |
| `0x00400000` | `0x00500000` |       Kernel stack        |
| `0x03000000` | `0x03??????` |        VESA buffer        |
| `0x40000000` | `0x????????` |    Userspace (virtual)    |

## Notes

I've designed this memory map with future expansion in mind, that's why there are some gaps in the map.

All physical kernel memory ranges are identity-mapped into the virtual space.

The free virtual space below the virtual userspace boundary is used for the kernel heap. The free virtual space above the virtual userspace boundary is used for the userspace programs, libraries, heap and stack.

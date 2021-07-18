// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef PROTOCOLS_MULTIBOOT1_H
#define PROTOCOLS_MULTIBOOT1_H

#include <kernel.h>

#define MULTIBOOT1_MAGIC 0x2badb002 // Passed to kernel

#define MULTIBOOT1_INFO_MEMORY 0x00000001
#define MULTIBOOT1_INFO_BOOTDEV 0x00000002
#define MULTIBOOT1_INFO_CMDLINE 0x00000004
#define MULTIBOOT1_INFO_MODS 0x00000008
#define MULTIBOOT1_INFO_AOUT_SYMS 0x00000010
#define MULTIBOOT1_INFO_ELF_SHDR 0x00000020
#define MULTIBOOT1_INFO_MEMORY_MAP 0x00000040
#define MULTIBOOT1_INFO_DRIVE_INFO 0x00000080
#define MULTIBOOT1_INFO_CONFIG_TABLE 0x00000100
#define MULTIBOOT1_INFO_BOOT_LOADER_NAME 0x00000200
#define MULTIBOOT1_INFO_APM_TABLE 0x00000400
#define MULTIBOOT1_INFO_VBE_INFO 0x00000800
#define MULTIBOOT1_INFO_FRAMEBUFFER_INFO 0x00001000

#define MULTIBOOT1_FRAMEBUFFER_TYPE_INDEXED 0
#define MULTIBOOT1_FRAMEBUFFER_TYPE_RGB 1
#define MULTIBOOT1_FRAMEBUFFER_TYPE_EGA_TEXT 2

#define MULTIBOOT1_MEMORY_AVAILABLE 1
#define MULTIBOOT1_MEMORY_RESERVED 2
#define MULTIBOOT1_MEMORY_ACPI_RECLAIMABLE 3
#define MULTIBOOT1_MEMORY_NVS 4
#define MULTIBOOT1_MEMORY_BADRAM 5

struct multiboot1_aout_symbol_table {
	u32 tabsize;
	u32 strsize;
	u32 addr;
	u32 reserved;
};

struct multiboot1_elf_section_header_table {
	u32 num;
	u32 size;
	u32 addr;
	u32 shndx;
};

struct multiboot1_info {
	u32 flags;

	u32 memory_lower;
	u32 memory_upper;

	u32 boot_device;

	u32 cmdline;

	u32 mods_count;
	u32 mods_addr;

	union {
		struct multiboot1_aout_symbol_table aout_sym;
		struct multiboot1_elf_section_header_table elf_sec;
	} u;

	u32 mmap_length;
	u32 mmap_addr;

	u32 drives_length;
	u32 drives_addr;

	u32 config_table;

	u32 boot_loader_name;

	u32 apm_table;

	u32 vbe_control_info;
	u32 vbe_mode_info;
	u16 vbe_mode;
	u16 vbe_interface_seg;
	u16 vbe_interface_off;
	u16 vbe_interface_len;

	u32 framebuffer_addr_high;
	u32 framebuffer_addr_low;
	u32 framebuffer_pitch;
	u32 framebuffer_width;
	u32 framebuffer_height;
	u8 framebuffer_bpp;
	u8 framebuffer_type;
	union {
		struct {
			u32 framebuffer_palette_addr;
			u16 framebuffer_palette_num_colors;
		} palette;
		struct {
			u8 framebuffer_red_field_position;
			u8 framebuffer_red_mask_size;
			u8 framebuffer_green_field_position;
			u8 framebuffer_green_mask_size;
			u8 framebuffer_blue_field_position;
			u8 framebuffer_blue_mask_size;
		} colors;
	} framebuffer_colors;
} PACKED;

struct multiboot1_mmap_entry {
	u32 struct_size;
	u32 addr_low;
	u32 addr_high;
	u32 len_low;
	u32 len_high;
	u32 type;
} PACKED;

struct multiboot1_mod_list {
	u32 mod_start;
	u32 mod_end;

	u32 cmdline;

	u32 pad;
};

u8 multiboot1_detect(u32 magic, uintptr_t address);
struct boot_information *multiboot1_convert(uintptr_t address, struct boot_information *info);

#endif

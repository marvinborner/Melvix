// MIT License, Copyright (c) 2021 Marvin Borner
// Useful macros for kernel

#ifndef KERNEL_H
#define KERNEL_H

#include <stddef.h>
#include <stdint.h>

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

#define EBDA ((uintptr_t)(*((u16 *)0x40e)) * 16)
#define BIOS ((uintptr_t)0xe0000)

#define ATTR __attribute__
#define PACKED ATTR((packed))
#define NOINLINE ATTR((noinline))
#define NONNULL ATTR((nonnull))
#define NORETURN ATTR((noreturn))
#define FLATTEN ATTR((flatten))
#define HOT ATTR((hot))
#define OPTIMIZE(level) ATTR((optimize(level)))
#define ALIGNED(align) ATTR((aligned(align)))

#define STRINGIFY_PARAM(a) #a
#define STRINGIFY(a) STRINGIFY_PARAM(a)
#define ALIGN_UP(addr, align) (((addr) + (align)-1) & ~((align)-1))
#define ALIGN_DOWN(addr, align) ((addr) & ~((align)-1))

#define CLEAR NOINLINE ATTR((section(".temp_clear")))
#define PROTECTED ATTR((section(".temp_protect")))

struct boot_information {
	struct {
		size_t total;
		struct {
			u8 available;
			void *entries;
			size_t count;
		} map;
	} memory;
	struct {
		u8 available;
		uintptr_t address;
		size_t width;
		size_t height;
		size_t pitch;
		size_t bpp;
	} framebuffer;
	struct {
		u8 available;
		uintptr_t rsdp;
	} acpi;
};

void kernel_panic(const char *reason);
void kernel_main(struct boot_information *data);

#endif

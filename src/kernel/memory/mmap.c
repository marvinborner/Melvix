#include <memory/mmap.h>
#include <stddef.h>
#include <stdint.h>
#include <system.h>

#define FRAME_SIZE 0x1000
#define FRAME_COUNT (total_memory / 4)
#define FRAME_TABLE_SIZE (FRAME_COUNT / 32)

#define FREE 0x0
#define USED 0x1

void *kmalloc(u32 size);

extern u32 kernel_end;
u32 kernel_end_address = (u32)&kernel_end;

u32 total_memory;

u32 *frame_table;

void *kmalloc(u32 size)
{
	void *ret;

	ret = (void *)kernel_end_address;
	kernel_end_address += size;

	return ret;
}

// TODO: Efficiency
void *kmalloc_frames(u32 num)
{
	u8 found;

	for (u32 i = 0; i < FRAME_COUNT; i++) {
		found = 1;
		for (u32 j = 0; j < num; j++) {
			if (mmap_index_check(i + j) == USED) {
				found = 0;
				break;
			}
		}
		if (found) {
			for (u32 j = 0; j < num; j++) {
				mmap_index_set_used(i + j);
			}
			return (void *)(i * FRAME_SIZE);
		}
	}

	warn("Not enough frames");
	return NULL;
}

void kfree_frames(void *ptr, u32 num)
{
	u32 address = (u32)ptr;
	u32 i = address;

	while (i < address + (num * FRAME_SIZE)) {
		mmap_address_set_free(address);
		i += FRAME_SIZE;
	}
}

u8 mmap_index_check(u32 n)
{
	return (frame_table[n / 32] >> (n % 32)) & 0x1;
}

void mmap_init(u32 size)
{
	total_memory = size;
	frame_table = kmalloc(FRAME_TABLE_SIZE);
}

void mmap_init_finalize()
{
	// TODO: Efficiency
	//memset(frame_table, 0xFF, (&kernel_end / FRAME_SIZE) / 8);
	for (u32 i = 0; i < kernel_end_address; i += FRAME_SIZE) {
		mmap_address_set_used(i);
	}
}

void mmap_address_set_free(u32 address)
{
	frame_table[(address / FRAME_SIZE) / 32] &= ~(1 << ((address / FRAME_SIZE) % 32));
}

void mmap_address_set_used(u32 address)
{
	frame_table[(address / FRAME_SIZE) / 32] |= (1 << ((address / FRAME_SIZE) % 32));
}

void mmap_index_set_free(u32 n)
{
	frame_table[n / 32] &= ~(1 << (n % 32));
}

void mmap_index_set_used(u32 n)
{
	frame_table[n / 32] |= 1 << (n % 32);
}
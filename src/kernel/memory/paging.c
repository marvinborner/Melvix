#include <lib/lib.h>
#include <memory/alloc.h>
#include <memory/paging.h>
#include <stdint.h>
#include <system.h>

u32 page_directory[1024] __attribute__((aligned(4096)));
u32 page_tables[1024][1024] __attribute__((aligned(4096)));

void paging_init()
{
	for (u32 i = 0; i < PAGE_COUNT; i++) {
		for (u32 j = 0; j < PAGE_COUNT; j++) {
			page_tables[i][j] = ((j * PAGE_ALIGN) + (i * PAGE_SIZE)) | PT_RW | PT_USER;
		}
	}

	for (u32 i = 0; i < PAGE_COUNT; i++) {
		page_directory[i] = (u32)page_tables[i] | PD_RW | PD_PRESENT | PD_USER;
	}
}

void paging_install()
{
	paging_init();

	if (!memory_init())
		paging_set_present(0, memory_get_all() >> 3);
	paging_set_used(0, ((u32)&kernel_end >> 12) + 1);

	u32 cr4;
	asm volatile("mov %%cr4, %0" : "=r"(cr4));
	cr4 &= 0xffffffef;
	asm volatile("mov %0, %%cr4" ::"r"(cr4));
	paging_enable();
	log("Installed paging");

	// Test!
	u32 *a = malloc(PAGE_ALIGN);
	u32 *b = malloc(PAGE_ALIGN);
	free(b);
	free(a);
	u32 *c = malloc(2048);
	assert(a == c);
	c[42] = 0x4242;
	assert(c[42] == 0x4242);
	free(c);
	info("Malloc test succeeded!");
}

u32 **paging_make_directory()
{
	u32 **dir = valloc(PAGE_COUNT * sizeof(*dir));
	dir[0] = valloc(PAGE_COUNT * PAGE_COUNT * sizeof(u32));
	for (int i = 1; i < PAGE_COUNT; i++)
		dir[i] = dir[0] + i * PAGE_COUNT;

	paging_init(dir);

	return dir;
}

void paging_remove_directory(u32 **dir)
{
	// TODO: Fix freeing of directory by calculating aligned offset
	free(dir[0]);
	free(dir);
}

void paging_disable()
{
	u32 cr0;
	asm("mov %%cr0, %0" : "=r"(cr0));
	cr0 &= 0x7fffffff;
	asm("mov %0, %%cr0" ::"r"(cr0));
	paging_enabled = 0;
}

void paging_enable()
{
	asm("mov %0, %%cr3" ::"r"(page_directory));
	u32 cr0;
	asm("mov %%cr0, %0" : "=r"(cr0));
	cr0 |= 0x80000000;
	asm("mov %0, %%cr0" ::"r"(cr0));
	paging_enabled = 1;
}

void paging_switch_directory(u32 **dir)
{
	asm("mov %0, %%cr3" ::"r"(dir));
}

void invlpg(u32 addr)
{
	asm("invlpg (%0)" ::"r"(addr) : "memory");
}

void paging_map(u32 phy, u32 virt, u16 flags)
{
	u32 pdi = virt >> 22;
	u32 pti = virt >> 12 & 0x03FF;
	page_tables[pdi][pti] = phy | flags;
	invlpg(virt);
}

u32 paging_get_phys(u32 virt)
{
	u32 pdi = virt >> 22;
	u32 pti = (virt >> 12) & 0x03FF;
	return page_tables[pdi][pti] & 0xFFFFF000;
}

u16 paging_get_flags(u32 virt)
{
	u32 pdi = virt >> 22;
	u32 pti = (virt >> 12) & 0x03FF;
	return page_tables[pdi][pti] & 0xFFF;
}

void paging_set_flag_up(u32 virt, u32 count, u32 flag)
{
	u32 page_n = virt / PAGE_ALIGN;
	for (u32 i = page_n; i < page_n + count; i++) {
		page_tables[i / PAGE_COUNT][i % PAGE_COUNT] |= flag;
		invlpg(i * PAGE_ALIGN);
	}
}

void paging_set_flag_down(u32 virt, u32 count, u32 flag)
{
	u32 page_n = virt / PAGE_ALIGN;
	for (u32 i = page_n; i < page_n + count; i++) {
		page_tables[i / PAGE_COUNT][i % PAGE_COUNT] &= ~flag;
		invlpg(i * PAGE_ALIGN);
	}
}

void paging_set_present(u32 virt, u32 count)
{
	paging_set_flag_up(virt, count, PT_PRESENT);
}

void paging_set_absent(u32 virt, u32 count)
{
	paging_set_flag_down(virt, count, PT_PRESENT);
}

void paging_set_used(u32 virt, u32 count)
{
	paging_set_flag_up(virt, count, PT_USED);
}

void paging_set_free(u32 virt, u32 count)
{
	paging_set_flag_down(virt, count, PT_USED);
}

u32 paging_find_pages(u32 count)
{
	u32 continuous = 0;
	u32 start_dir = 0;
	u32 start_page = 0;
	for (u32 i = 0; i < PAGE_COUNT; i++) {
		for (u32 j = 0; j < PAGE_COUNT; j++) {
			if (!(page_tables[i][j] & PT_PRESENT) || (page_tables[i][j] & PT_USED)) {
				continuous = 0;
				start_dir = i;
				start_page = j + 1;
			} else {
				if (++continuous == count)
					return (start_dir * PAGE_SIZE) + (start_page * PAGE_ALIGN);
			}
		}
	}

	panic("Out of memory!");
	return 0;
}

u32 paging_alloc_pages(u32 count)
{
	u32 ptr = paging_find_pages(count);
	paging_set_used(ptr, count);
	return ptr;
}

u32 paging_get_used_pages()
{
	u32 n = 0;
	for (u32 i = 0; i < PAGE_COUNT; i++) {
		for (u32 j = 0; j < PAGE_COUNT; j++) {
			u8 flags = page_tables[i][j] & PT_USED;
			if (flags == 1)
				n++;
		}
	}
	return n;
}
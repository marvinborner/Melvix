#include <lib/lib.h>
#include <memory/alloc.h>
#include <memory/paging.h>
#include <stdint.h>
#include <system.h>

u32 *current_page_directory;
u32 (*current_page_tables)[1024];
u32 kernel_page_directory[1024] __attribute__((aligned(4096)));
u32 kernel_page_tables[1024][1024] __attribute__((aligned(4096)));

void paging_init(u32 *dir, int user)
{
	for (u32 i = 0; i < 1024; i++) {
		for (u32 j = 0; j < 1024; j++) {
			current_page_tables[i][j] = ((j * 0x1000) + (i * 0x400000)) | PT_RW;
		}
	}

	for (u32 i = 0; i < 1024; i++) {
		current_page_directory[i] = ((u32)current_page_tables[i]) | PD_RW | PD_PRESENT;
	}
}

extern void KERNEL_END();
void paging_install(u32 multiboot_address)
{
	paging_switch_directory(kernel_page_directory);
	paging_init(current_page_directory, 0);

	// if mmap approach didn't work
	if (!memory_init(multiboot_address))
		paging_set_present(0, memory_get_all() >> 3); // /4
	paging_set_used(0, ((u32)KERNEL_END >> 12) + 1); // /4096
	// paging_set_user(0, memory_get_all() >> 3); // HMM

	paging_enable();
	log("Installed paging");
}

u32 *paging_make_directory(int user)
{
	u32 *dir = malloc(1024 * 1024 * 32);

	paging_init(dir, user);

	return dir;
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
	asm("mov %0, %%cr3" ::"r"(current_page_directory));
	u32 cr0;
	asm("mov %%cr0, %0" : "=r"(cr0));
	cr0 |= 0x80000000;
	asm("mov %0, %%cr0" ::"r"(cr0));
	paging_enabled = 1;
}

void paging_switch_directory(u32 *dir)
{
	current_page_tables = kernel_page_tables;
	current_page_directory = dir;
	asm("mov %0, %%cr3" ::"r"(current_page_directory));
}

void invlpg(u32 addr)
{
	asm("invlpg (%0)" ::"r"(addr) : "memory");
}

void paging_map(u32 phy, u32 virt, u16 flags)
{
	u32 pdi = virt >> 22;
	u32 pti = virt >> 12 & 0x03FF;
	current_page_tables[pdi][pti] = phy | flags;
	invlpg(virt);
}

u32 paging_get_phys(u32 virt)
{
	u32 pdi = virt >> 22;
	u32 pti = (virt >> 12) & 0x03FF;
	return current_page_tables[pdi][pti] & 0xFFFFF000;
}

u16 paging_get_flags(u32 virt)
{
	u32 pdi = virt >> 22;
	u32 pti = (virt >> 12) & 0x03FF;
	return current_page_tables[pdi][pti] & 0xFFF;
}

void paging_set_flag_up(u32 virt, u32 count, u32 flag)
{
	u32 page_n = virt / 0x1000;
	for (u32 i = page_n; i < page_n + count; i++) {
		current_page_tables[i / 1024][i % 1024] |= flag;
		invlpg(i * 0x1000);
	}
}

void paging_set_flag_down(u32 virt, u32 count, u32 flag)
{
	u32 page_n = virt / 0x1000;
	for (u32 i = page_n; i < page_n + count; i++) {
		current_page_tables[i / 1024][i % 1024] &= ~flag;
		invlpg(i * 0x1000);
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

void paging_set_user(u32 virt, u32 count)
{
	u32 page_n = virt / 0x1000;
	for (u32 i = page_n; i < page_n + count; i += 1024) {
		current_page_directory[i / 1024] |= PD_USER;
	}
	paging_set_flag_up(virt, count, PT_USER);
}

u32 paging_find_pages(u32 count)
{
	u32 continuous = 0;
	u32 start_dir = 0;
	u32 start_page = 0;
	for (u32 i = 0; i < 1024; i++) {
		for (u32 j = 0; j < 1024; j++) {
			if (!(current_page_tables[i][j] & PT_PRESENT) ||
			    (current_page_tables[i][j] & PT_USED)) {
				continuous = 0;
				start_dir = i;
				start_page = j + 1;
			} else {
				if (++continuous == count)
					return (start_dir * 0x400000) + (start_page * 0x1000);
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
	paging_set_user(ptr, count);
	return ptr;
}

u32 paging_get_used_pages()
{
	u32 n = 0;
	for (u32 i = 0; i < 1024; i++) {
		for (u32 j = 0; j < 1024; j++) {
			u8 flags = current_page_tables[i][j] & PT_USED;
			if (flags == 1)
				n++;
		}
	}
	return n;
}

#include <lib/lib.h>
#include <memory/alloc.h>
#include <memory/paging.h>
#include <stdint.h>
#include <system.h>

int paging_enabled = 0;

struct page_directory *paging_directory; // Current
struct page_directory *paging_kernel_directory = 0x9d000;

void paging_init(struct page_directory *dir, int user)
{
	for (u32 i = 0; i < 1024; i++) {
		for (u32 j = 0; j < 1024; j++) {
			dir->tables[i]->pages[j] =
				((j * 0x1000) + (i * PAGE_SIZE)) | PT_RW | (user ? PT_USER : 0);
		}
	}

	for (u32 i = 0; i < 1024; i++) {
		dir->tables[i] = ((u32)dir->tables[i]) | PD_RW | PD_PRESENT | (user ? PD_USER : 0);
	}
}

struct page_directory *paging_make_directory(int user)
{
	struct page_directory *dir = (struct page_directory *)malloc(sizeof(struct page_directory));

	paging_init(dir, user);

	return dir;
}

extern void KERNEL_END();
void paging_install(u32 multiboot_address)
{
	paging_switch_directory(paging_kernel_directory);
	paging_init(paging_directory, 0);

	if (!memory_init(multiboot_address))
		paging_set_present(0, memory_get_all() >> 3);

	paging_set_used(0, ((u32)KERNEL_END >> 12) + 1); // /4096
	log("Enabling");

	paging_enable();
	log("Installed paging");
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
	u32 cr0;
	asm("mov %%cr0, %0" : "=r"(cr0));
	cr0 |= 0x80000000;
	asm("mov %0, %%cr0" ::"r"(cr0));
	paging_enabled = 1;
}

void paging_switch_directory(struct page_directory *dir)
{
	paging_directory = dir;
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
	paging_directory->tables[pdi]->pages[pti] = phy | flags;
	invlpg(virt);
}

u32 paging_get_phys(u32 virt)
{
	u32 pdi = virt >> 22;
	u32 pti = (virt >> 12) & 0x03FF;
	return paging_directory->tables[pdi]->pages[pti] & 0xFFFFF000;
}

u16 paging_get_flags(u32 virt)
{
	u32 pdi = virt >> 22;
	u32 pti = (virt >> 12) & 0x03FF;
	return paging_directory->tables[pdi]->pages[pti] & 0xFFF;
}

void paging_set_flag_up(u32 virt, u32 count, u32 flag)
{
	u32 page_n = virt / 0x1000;
	for (u32 i = page_n; i < page_n + count; i++) {
		paging_directory->tables[i / 1024]->pages[i % 1024] |= flag;
		invlpg(i * 0x1000);
	}
}

void paging_set_flag_down(u32 virt, u32 count, u32 flag)
{
	u32 page_n = virt / 0x1000;
	for (u32 i = page_n; i < page_n + count; i++) {
		paging_directory->tables[i / 1024]->pages[i % 1024] &= ~flag;
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
		paging_directory->tables[i] = ((u32)paging_directory->tables[i]) | PD_USER;
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
			if (!(paging_directory->tables[i]->pages[j] & PT_PRESENT) ||
			    (paging_directory->tables[i]->pages[j] & PT_USED)) {
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
			u8 flags = paging_directory->tables[i]->pages[j] & PT_USED;
			if (flags == 1)
				n++;
		}
	}
	return n;
}
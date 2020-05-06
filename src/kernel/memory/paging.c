#include <stdint.h>
#include <memory/paging.h>
#include <memory/alloc.h>
#include <system.h>
#include <lib/lib.h>
#include <io/io.h>
#include <acpi/acpi.h>
#include <tasks/process.h>
#include <interrupts/interrupts.h>

struct page_directory *paging_current_directory = NULL;
struct page_directory *paging_root_directory = NULL;

/* void paging_install(u32 multiboot_address) */
/* { */
/* 	if (!memory_init(multiboot_address)) */
/* 		paging_set_present(0, memory_get_all() >> 3); // /4 */
/* 	paging_set_used(0, ((u32)ASM_KERNEL_END >> 12) + 1); // /4096 */
/* } */

struct page_table *get_cr3()
{
	u32 cr3;
	asm volatile("movl %%cr3, %%eax" : "=a"(cr3));
	return (struct page_table *)cr3;
}

u32 get_cr0()
{
	u32 cr0;
	asm volatile("movl %%cr0, %%eax" : "=a"(cr0));
	return cr0;
}

void set_cr3(struct page_directory *dir)
{
	u32 addr = (u32)&dir->tables[0];
	asm volatile("movl %%eax, %%cr3" ::"a"(addr));
}

void set_cr0(u32 cr0)
{
	asm volatile("movl %%eax, %%cr0" ::"a"(cr0));
}

void paging_disable()
{
	set_cr0(get_cr0() | 0x7fffffff);
}

void paging_enable()
{
	set_cr3(paging_current_directory);
	set_cr0(get_cr0() | 0x80000000);
}

void paging_switch_directory(struct page_directory *dir)
{
	set_cr3(dir);
	set_cr0(get_cr0() | 0x80000000);
}

struct page_directory *paging_make_directory()
{
	struct page_directory *dir =
		(struct page_directory *)kmalloc_a(sizeof(struct page_directory));

	for (int i = 0; i < 1024; i++)
		dir->tables[i] = EMPTY_TAB;

	return dir;
}

struct page_table *paging_make_table()
{
	struct page_table *tab = (struct page_table *)kmalloc_a(sizeof(struct page_table));

	for (int i = 0; i < 1024; i++) {
		tab->pages[i].present = 0;
		tab->pages[i].rw = 1;
	}

	return tab;
}

void paging_map(struct page_directory *dir, u32 phys, u32 virt)
{
	short id = virt >> 22;
	struct page_table *tab = paging_make_table();

	dir->tables[id] = ((struct page_table *)((u32)tab | 3)); // RW

	for (int i = 0; i < 1024; i++) {
		tab->pages[i].frame = phys >> 12;
		tab->pages[i].present = 1;
		phys += 4096;
	}
}

void paging_map_user(struct page_directory *dir, u32 phys, u32 virt)
{
	short id = virt >> 22;
	struct page_table *tab = paging_make_table();

	dir->tables[id] = ((struct page_table *)((u32)tab | 3 | 4)); // RW + usermode

	for (int i = 0; i < 1024; i++) {
		tab->pages[i].frame = phys >> 12;
		tab->pages[i].present = 1;
		tab->pages[i].user = 1;
		phys += 4096;
	}
}

// Hmm
u32 paging_get_phys(u32 virt)
{
	u32 pdi = virt >> 22;
	u32 pti = (virt >> 12) & 0x03FF;
	return (*(u32 *)&paging_current_directory->tables[pdi]->pages[pti]) & 0xFFFFF000;
}

void paging_install()
{
	kheap_init();

	paging_current_directory = paging_make_directory();
	paging_root_directory = paging_current_directory;

	for (u32 i = 0; i < 0xF0000000; i += PAGE_S)
		paging_map(paging_root_directory, i, i);
	paging_switch_directory(paging_root_directory);
	info("Installed paging");

	// Test mallocing
	u32 a = (u32)kmalloc(4096);
	u32 b = (u32)kmalloc(4096);
	kfree((void *)b);
	kfree((void *)a);
	u32 c = (u32)kmalloc(2048);
	assert(a == c);
	info("kmalloc test succeeded!");
}

void paging_convert_page(struct page_directory *kdir)
{
	for (int i = 0; i < 1024; i++) {
		kdir->tables[i] = (struct page_table *)((u32)kdir->tables[i] | 4); // Usermode

		if (((u32)kdir->tables[i]) & 1) { // Is present
			for (int j = 0; j < 1024; j++)
				kdir->tables[i]->pages[j].user = 1; // Usermode
		}
	}
}

struct page_directory *paging_copy_user_directory(struct page_directory *dir)
{
	struct page_directory *copy = paging_make_directory();
	memcpy(copy, paging_root_directory, sizeof(struct page_directory));

	for (u32 i = 0; i < 1024; i++) {
		if (((u32)dir->tables[i]) & 4) {
			struct page_table *tab =
				(struct page_table *)((u32)dir->tables[i] & 0xFFFFF000);

			void *buffer = kmalloc_a(PAGE_S);
			memcpy(buffer, (void *)(tab->pages[0].frame << 12), PAGE_S);
			paging_map_user(copy, (u32)buffer, (u32)i << 22);
		}
	}

	return copy;
}
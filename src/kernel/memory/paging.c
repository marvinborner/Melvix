#include <io/io.h>
#include <lib/lib.h>
#include <memory/mmap.h>
#include <memory/paging.h>
#include <stdint.h>
#include <system.h>

struct page_dir *kernel_page_directory;

void paging_install()
{
	memory_init();

	struct page_table *page_table;

	page_table = kmalloc_frames(1);
	memset(page_table, 0, sizeof(struct page_table));
	for (u32 i = 0; i < PAGE_COUNT; i++) {
		page_table->entries[i].present = 1;
		page_table->entries[i].writable = 1;
		page_table->entries[i].address = SHIFT(i * PAGE_SIZE);
	}

	kernel_page_directory = kmalloc_frames(1);
	memset(kernel_page_directory, 0, sizeof(struct page_dir));
	kernel_page_directory->entries[0].present = 1;
	kernel_page_directory->entries[0].writable = 1;
	kernel_page_directory->entries[0].address = SHIFT((u32)page_table);

	paging_switch_directory((u32)kernel_page_directory);
	paging_enable();
	info("Installed paging");
}

void paging_disable()
{
	u32 cr0 = cr0_get();
	cr0 &= 0x7fffffff;
	cr0_set(cr0);
	paging_enabled = 0;
}

void paging_enable()
{
	u32 cr0 = cr0_get();
	cr0 |= 0x80000000;
	cr0_set(cr0);
	paging_enabled = 1;
}

void paging_switch_directory(u32 dir)
{
	cr3_set(dir);
}

struct page_table_entry *paging_get_page(u32 address, struct page_dir *page_dir)
{
	struct page_table *page_table;

	address /= PAGE_SIZE;
	u32 n = address / PAGE_COUNT;

	if (page_dir->entries[n].present == 0) {
		page_table = kmalloc_frames(1);
		memset(page_table, 0, sizeof(struct page_table));

		page_dir->entries[n].address = SHIFT((u32)page_table);
		page_dir->entries[n].present = 1;
		page_dir->entries[n].writable = 1;
		page_dir->entries[n].user = 1;
	} else {
		page_table = (void *)UNSHIFT(page_dir->entries[n].address);
	}

	return &page_table->entries[address % PAGE_COUNT];
}

void paging_frame_alloc(struct page_table_entry *page)
{
	void *ptr = kmalloc_frames(1);

	if (page->address != 0) {
		warn("Page is already allocated");
		return;
	}
	page->address = SHIFT((u32)ptr);
	page->present = 1;
	page->user = 1;
}

void paging_frame_free(struct page_table_entry *page)
{
	kfree_frames((void *)UNSHIFT(page->address), 1);
	memset((void *)page, 0, sizeof(struct page_table_entry));
}

struct page_dir *paging_make_dir()
{
	struct page_dir *ret = kmalloc_frames(1);

	memcpy(ret, kernel_page_directory, sizeof(struct page_dir));

	return ret;
}

void paging_free_dir(struct page_dir *page_dir)
{
	kfree_frames(page_dir, 1);
}
#include <stdint.h>
#include <kernel/paging/paging.h>
#include <kernel/system.h>
#include <kernel/lib/lib.h>

int paging_enabled = 0;
uint32_t page_directory[1024] __attribute__((aligned(4096)));
uint32_t page_tables[1024][1024] __attribute__((aligned(4096)));

void paging_install()
{
    for (uint32_t i = 0; i < 1024; i++) {
        for (uint32_t j = 0; j < 1024; j++) {
            page_tables[i][j] = ((j * 0x1000) + (i * 0x400000)) | PT_RW;
        }
    }

    for (uint32_t i = 0; i < 1024; i++) {
        page_directory[i] = ((uint32_t) page_tables[i]) | PD_RW | PD_PRESENT;
    }

    paging_set_present(0, memory_get_all() >> 2); // /4
    paging_set_used(0, ((uint32_t) ASM_KERNEL_END >> 12) + 1); // /4096

    paging_enable();

    vga_log("Installed paging");
}

void paging_disable()
{
    uint32_t cr0;
    asm ("mov %%cr0, %0": "=r"(cr0));
    cr0 &= 0x7fffffff;
    asm ("mov %0, %%cr0"::"r"(cr0));
    paging_enabled = 0;
}

void paging_enable()
{
    asm ("mov %0, %%cr3"::"r"(page_directory));
    uint32_t cr0;
    asm ("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000;
    asm ("mov %0, %%cr0"::"r"(cr0));
    paging_enabled = 1;
}

inline void invlpg(uint32_t addr)
{
    asm ("invlpg (%0)"::"r" (addr) : "memory");
}

void paging_map(uint32_t phy, uint32_t virt, uint16_t flags)
{
    uint32_t pdi = virt >> 22;
    uint32_t pti = virt >> 12 & 0x03FF;
    page_tables[pdi][pti] = phy | flags;
    invlpg(virt);
}

uint32_t paging_get_phys(uint32_t virt)
{
    uint32_t pdi = virt >> 22;
    uint32_t pti = (virt >> 12) & 0x03FF;
    return page_tables[pdi][pti] & 0xFFFFF000;
}

uint16_t paging_get_flags(uint32_t virt)
{
    uint32_t pdi = virt >> 22;
    uint32_t pti = (virt >> 12) & 0x03FF;
    return page_tables[pdi][pti] & 0xFFF;
}

void paging_set_flag_up(uint32_t virt, uint32_t count, uint32_t flag)
{
    uint32_t page_n = virt / 4096;
    for (uint32_t i = page_n; i < page_n + count; i++) {
        page_tables[i / 1024][i % 1024] |= flag;
        invlpg(i * 4096);
    }
}

void paging_set_flag_down(uint32_t virt, uint32_t count, uint32_t flag)
{
    uint32_t page_n = virt / 4096;
    for (uint32_t i = page_n; i < page_n + count; i++) {
        page_tables[i / 1024][i % 1024] &= ~flag;
        invlpg(i * 4096);
    }
}

void paging_set_present(uint32_t virt, uint32_t count)
{
    paging_set_flag_up(virt, count, PT_PRESENT);
}

void paging_set_absent(uint32_t virt, uint32_t count)
{
    paging_set_flag_down(virt, count, PT_PRESENT);
}

void paging_set_used(uint32_t virt, uint32_t count)
{
    paging_set_flag_up(virt, count, PT_USED);
}

void paging_set_free(uint32_t virt, uint32_t count)
{
    paging_set_flag_down(virt, count, PT_USED);
}

void paging_set_user(uint32_t virt, uint32_t count)
{
    uint32_t page_n = virt / 4096;
    for (uint32_t i = page_n; i < page_n + count; i += 1024) {
        page_directory[i / 1024] |= PD_ALL_PRIV;
    }
    paging_set_flag_up(virt, count, PT_ALL_PRIV);
}

uint32_t paging_find_pages(uint32_t count)
{
    uint32_t continuous = 0;
    uint32_t startDir = 0;
    uint32_t startPage = 0;
    for (uint32_t i = 0; i < 1024; i++) {
        for (uint32_t j = 0; j < 1024; j++) {
            if (!(page_tables[i][j] & PT_PRESENT) || (page_tables[i][j] & PT_USED)) {
                continuous = 0;
                startDir = i;
                startPage = j + 1;
            } else {
                if (++continuous == count)
                    return (startDir * 0x400000) + (startPage * 0x1000);
            }
        }
    }

    panic("Out of memory!");
    return 0;
}

uint32_t paging_alloc_pages(uint32_t count)
{
    uint32_t ptr = paging_find_pages(count);
    paging_set_used(ptr, count);
    paging_set_user(ptr, count);
    return ptr;
}

uint32_t paging_get_used_pages()
{
    uint32_t n = 0;
    for (uint32_t i = 0; i < 1024; i++) {
        for (uint32_t j = 0; j < 1024; j++) {
            uint8_t flags = page_tables[i][j] & PT_USED;
            if (flags == 1) n++;
        }
    }
    return n;
}
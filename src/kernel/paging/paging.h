#ifndef MELVIX_PAGING_H
#define MELVIX_PAGING_H

#include <stdint.h>

#define PD_PRESENT  1 << 0
#define PD_RW       1 << 1
#define PD_ALL_PRIV 1 << 2
#define PD_WRITETHR 1 << 3
#define PD_CACHE_D  1 << 4
#define PD_ACCESSED 1 << 5
#define PD_4M_PAGE  1 << 7

#define PT_PRESENT  1 << 0
#define PT_RW       1 << 1
#define PT_ALL_PRIV 1 << 2
#define PT_WRITETHR 1 << 3
#define PT_CACHE_D  1 << 4
#define PT_ACCESSED 1 << 5
#define PT_DIRTY    1 << 6
#define PT_GLOBAL   1 << 8
#define PT_USED     1 << 9

void paging_install();

void paging_enable();

void paging_disable();

void paging_map(uint32_t phy, uint32_t virt, uint16_t flags);

uint32_t paging_get_phys(uint32_t virt);

uint16_t paging_get_flags(uint32_t virt);

void paging_set_flags(uint32_t virt, uint32_t count, uint16_t flags);

void paging_set_flag_up(uint32_t virt, uint32_t count, uint32_t flag);

void paging_set_flag_down(uint32_t virt, uint32_t count, uint32_t flag);

void paging_set_present(uint32_t virt, uint32_t count);

void paging_set_absent(uint32_t virt, uint32_t count);

void paging_set_used(uint32_t virt, uint32_t count);

void paging_set_free(uint32_t virt, uint32_t count);

void paging_set_user(uint32_t virt, uint32_t count);

uint32_t paging_find_pages(uint32_t count);

uint32_t paging_alloc_pages(uint32_t count);

uint32_t paging_get_used_pages();

#endif

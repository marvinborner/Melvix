#ifndef MELVIX_PAGING_H
#define MELVIX_PAGING_H

#define PAGE_S 0x400000

extern unsigned int *current_dir;
extern unsigned int *root_dir;

typedef struct {
    unsigned int present    : 1;
    unsigned int rw        : 1;
    unsigned int user    : 1;
    unsigned int accessed    : 1;
    unsigned int dirty    : 1;
    unsigned int unused    : 7;
    unsigned int frame    : 20;
} page_t;

typedef struct {
    page_t pages[1024];
} page_table_t;

typedef struct {
    page_table_t *tables[1024];
    unsigned int tables_physical[1024];
    unsigned int physical_address;
} page_directory_t;

typedef struct {
    page_table_t *tables[1024];
} vpage_dir_t;

extern vpage_dir_t *root_vpage_dir;

#define EMPTY_TAB ((page_table_t*) 0x00000002)

page_table_t *get_cr3();

unsigned int get_cr0();

void set_cr3(vpage_dir_t *dir);

void set_cr0(unsigned int new_cr0);

void switch_vpage_dir(vpage_dir_t *dir);

vpage_dir_t *mk_vpage_dir();

page_table_t *mk_vpage_table();

void page_init();

void vpage_map(vpage_dir_t *cr3, unsigned int virt, unsigned int phys);

void vpage_map_user(vpage_dir_t *cr3, unsigned int virt, unsigned int phys);

void convert_vpage(vpage_dir_t *kdir);

void dump_page(vpage_dir_t *dir, unsigned int address);

vpage_dir_t *copy_user_dir(vpage_dir_t *dir);

#endif

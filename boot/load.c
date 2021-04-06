// MIT License, Copyright (c) 2021 Marvin Borner
// Independent ext2 loader - mostly copied from kernel

/**
 * Some general definitions
 */

typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

#define NULL ((void *)0)
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#define PACKED __attribute__((packed))

#define assert(exp)                                                                                \
	if (!(exp)) {                                                                              \
		print(__FILE__);                                                                   \
		print(": ");                                                                       \
		print(__func__);                                                                   \
		print(": Bootloader assertion '");                                                 \
		print(#exp);                                                                       \
		print("' failed.\n");                                                              \
		__asm__ volatile("cli\nhlt");                                                      \
	}

/**
 * ATA numbers
 */

#define BLOCK_SIZE 1024
#define BLOCK_COUNT 256 // BLOCK_SIZE / sizeof(u32)
#define SECTOR_SIZE 512
#define SECTOR_COUNT (BLOCK_SIZE / SECTOR_SIZE)

#define ATA_PRIMARY_IO 0x1f0
#define ATA_SECONDARY_IO 0x170

#define ATA_PRIMARY 0x00
#define ATA_SECONDARY 0x01
#define ATA_READ 0x00
#define ATA_WRITE 0x013
#define ATA_MASTER 0x00
#define ATA_SLAVE 0x01
#define ATA_SR_BSY 0x80
#define ATA_SR_DRDY 0x40
#define ATA_SR_DF 0x20
#define ATA_SR_DSC 0x10
#define ATA_SR_DRQ 0x08
#define ATA_SR_CORR 0x04
#define ATA_SR_IDX 0x02
#define ATA_SR_ERR 0x01
#define ATA_REG_DATA 0x00
#define ATA_REG_ERROR 0x01
#define ATA_REG_FEATURES 0x01
#define ATA_REG_SECCOUNT0 0x02
#define ATA_REG_LBA0 0x03
#define ATA_REG_LBA1 0x04
#define ATA_REG_LBA2 0x05
#define ATA_REG_HDDEVSEL 0x06
#define ATA_REG_COMMAND 0x07
#define ATA_REG_STATUS 0x07
#define ATA_REG_SECCOUNT1 0x08
#define ATA_REG_LBA3 0x09
#define ATA_REG_LBA4 0x0a
#define ATA_REG_LBA5 0x0b
#define ATA_REG_CONTROL 0x0c
#define ATA_REG_ALTSTATUS 0x0c
#define ATA_REG_DEVADDRESS 0x0d
#define ATA_CMD_READ_PIO 0x20
#define ATA_CMD_READ_PIO_EXT 0x24
#define ATA_CMD_READ_DMA 0xc8
#define ATA_CMD_READ_DMA_EXT 0x25
#define ATA_CMD_WRITE_PIO 0x30
#define ATA_CMD_WRITE_PIO_EXT 0x34
#define ATA_CMD_WRITE_DMA 0xca
#define ATA_CMD_WRITE_DMA_EXT 0x35
#define ATA_CMD_CACHE_FLUSH 0xe7
#define ATA_CMD_CACHE_FLUSH_EXT 0xea
#define ATA_CMD_PACKET 0xa0
#define ATA_CMD_IDENTIFY_PACKET 0xa1
#define ATA_CMD_IDENTIFY 0xec
#define ATA_IDENT_DEVICETYPE 0
#define ATA_IDENT_CYLINDERS 2
#define ATA_IDENT_HEADS 6
#define ATA_IDENT_SECTORS 12
#define ATA_IDENT_SERIAL 20
#define ATA_IDENT_MODEL 54
#define ATA_IDENT_CAPABILITIES 98
#define ATA_IDENT_FIELDVALID 106
#define ATA_IDENT_MAX_LBA 120
#define ATA_IDENT_COMMANDSETS 164
#define ATA_IDENT_MAX_LBA_EXT 200

/**
 * ELF stuff
 */

#define ELF_MAG0 0x7F
#define ELF_MAG1 'E'
#define ELF_MAG2 'L'
#define ELF_MAG3 'F'

#define ELF_IDENT_COUNT 16
#define ELF_IDENT_MAG0 0
#define ELF_IDENT_MAG1 1
#define ELF_IDENT_MAG2 2
#define ELF_IDENT_MAG3 3

#define ELF_IDENT_CLASS 4
#define ELF_IDENT_CLASS_NONE 0
#define ELF_IDENT_CLASS_32 1
#define ELF_IDENT_CLASS_64 2

#define ELF_IDENT_DATA 5
#define ELF_IDENT_DATA_NONE 0
#define ELF_IDENT_DATA_LSB 1
#define ELF_IDENT_DATA_MSB 2

#define ELF_IDENT_VERSION 6
#define ELF_IDENT_OSABI 7
#define ELF_IDENT_ABIVERSION 8
#define ELF_IDENT_PAD 9

#define ELF_ETYPE_NONE 0
#define ELF_ETYPE_REL 1
#define ELF_ETYPE_EXEC 2
#define ELF_ETYPE_DYN 3
#define ELF_ETYPE_CORE 4
#define ELF_ETYPE_NUM 5

#define ELF_MACHINE_NONE 0
#define ELF_MACHINE_SPARC 2
#define ELF_MACHINE_386 3
#define ELF_MACHINE_SPARC32PLUS 18
#define ELF_MACHINE_SPARCV9 43
#define ELF_MACHINE_AMD64 62

#define ELF_PROGRAM_TYPE_NULL 0
#define ELF_PROGRAM_TYPE_LOAD 1
#define ELF_PROGRAM_TYPE_DYNAMIC 2
#define ELF_PROGRAM_TYPE_INTERP 3
#define ELF_PROGRAM_TYPE_NOTE 4
#define ELF_PROGRAM_TYPE_SHLIB 5
#define ELF_PROGRAM_TYPE_PHDR 6
#define ELF_PROGRAM_TYPE_TLS 7

#define ELF_PROGRAM_FLAG_X 0x1
#define ELF_PROGRAM_FLAG_W 0x2
#define ELF_PROGRAM_FLAG_R 0x4

#define ELF_SECTION_TYPE_NULL 0
#define ELF_SECTION_TYPE_PROGBITS 1
#define ELF_SECTION_TYPE_SYMTAB 2
#define ELF_SECTION_TYPE_STRTAB 3
#define ELF_SECTION_TYPE_RELA 4
#define ELF_SECTION_TYPE_HASH 5
#define ELF_SECTION_TYPE_DYNAMIC 6
#define ELF_SECTION_TYPE_NOTE 7
#define ELF_SECTION_TYPE_NOBITS 8
#define ELF_SECTION_TYPE_REL 9
#define ELF_SECTION_TYPE_SHLIB 10
#define ELF_SECTION_TYPE_DYNSYM 11
#define ELF_SECTION_TYPE_COUNT 12

#define ELF_SECTION_FLAG_WRITE 0x1
#define ELF_SECTION_FLAG_ALLOC 0x2
#define ELF_SECTION_FLAG_EXEC 0x3
#define ELF_SECTION_FLAG_MERGE 0x10
#define ELF_SECTION_FLAG_STRINGS 0x20
#define ELF_SECTION_FLAG_INFO_LINK 0x40
#define ELF_SECTION_FLAG_LINK_ORDER 0x80
#define ELF_SECTION_FLAG_OS_SPECIAL 0x100
#define ELF_SECTION_FLAG_GROUP 0x200
#define ELF_SECTION_FLAG_TLS 0x400
#define ELF_SECTION_FLAG_COMPRESSED 0x800

#define ELF_BSS ".bss"
#define ELF_DATA ".data"
#define ELF_DEBUG ".debug"
#define ELF_DYNAMIC ".dynamic"
#define ELF_DYNSTR ".dynstr"
#define ELF_DYNSYM ".dynsym"
#define ELF_FINI ".fini"
#define ELF_GOT ".got"
#define ELF_HASH ".hash"
#define ELF_INIT ".init"
#define ELF_REL_DATA ".rel.data"
#define ELF_REL_FINI ".rel.fini"
#define ELF_REL_INIT ".rel.init"
#define ELF_REL_DYN ".rel.dyn"
#define ELF_REL_RODATA ".rel.rodata"
#define ELF_REL_TEXT ".rel.text"
#define ELF_RODATA ".rodata"
#define ELF_SHSTRTAB ".shstrtab"
#define ELF_STRTAB ".strtab"
#define ELF_SYMTAB ".symtab"
#define ELF_TEXT ".text"

struct PACKED elf_header {
	u8 ident[ELF_IDENT_COUNT];
	u16 type;
	u16 machine;
	u32 version;
	u32 entry;
	u32 phoff;
	u32 shoff;
	u32 flags;
	u16 ehsize;
	u16 phentsize;
	u16 phnum;
	u16 shentsize;
	u16 shnum;
	u16 shstrndx;
};

struct PACKED elf_program {
	u32 type;
	u32 offset;
	u32 vaddr;
	u32 paddr;
	u32 filesz;
	u32 memsz;
	u32 flags;
	u32 align;
};

struct PACKED elf_section {
	u32 name;
	u32 type;
	u32 flags;
	u32 addr;
	u32 offset;
	u32 size;
	u32 link;
	u32 info;
	u32 addralign;
	u32 entsize;
};

struct PACKED elf_symbol {
	u32 name;
	u32 value;
	u32 size;
	u8 info;
	u8 other;
	u16 shndx;
};

/**
 * EXT2 numbers/structs
 */

#define EXT2_BOOT 0
#define EXT2_SUPER 1
#define EXT2_ROOT 2
#define EXT2_MAGIC 0x0000EF53

struct ext2_superblock {
	u32 total_inodes;
	u32 total_blocks;
	u32 su_res_blocks; // Superuser reserved
	u32 free_blocks;
	u32 free_inodes;
	u32 superblock_block_num;
	u32 log2_block_size;
	u32 log2_frag_size;
	u32 blocks_per_group;
	u32 frags_per_group;
	u32 inodes_per_group;
	u32 last_mount_time;
	u32 last_write_time;
	u16 mounts_since_fsck;
	u16 max_mounts_since_fsck;
	u16 magic;
	u16 state; // 1 clean; 2 errors
	u16 error_action;
	u16 minor_version;
	u32 last_fsck_time;
	u32 max_time_since_fsck;
	u32 creator_os_id;
	u32 major_version;
	u16 res_block_uid;
	u16 res_block_gid;
};

struct ext2_bgd {
	u32 block_bitmap;
	u32 inode_bitmap;
	u32 inode_table;
	u16 free_blocks;
	u16 free_inodes;
	u16 used_dirs;
	u16 pad;
	u8 bg_reserved[12];
};

struct ext2_inode {
	u16 mode;
	u16 uid;
	u32 size;

	u32 last_access_time;
	u32 creation_time;
	u32 last_modification_time;
	u32 deletion_time;

	u16 gid;
	u16 link_count;
	u32 blocks;
	u32 flags;
	u32 os_specific_val1;
	u32 block[15];
	u32 generation;

	u32 reserved1;
	u32 reserved2;

	u32 fragment_addr;
	u8 os_specific_val2[12];
};

#define EXT2_INODE_SIZE (sizeof(struct ext2_inode))

struct ext2_dirent {
	u32 inode_num;
	u16 total_len;
	u8 name_len;
	u8 type_indicator;
	u8 name[];
};

struct ext2_file {
	struct ext2_inode inode;
	u32 pos;
	u8 block_index;
	u8 *buf;
	u32 curr_block_pos;
};

/**
 * Memory
 */

static u32 heap = 0x0000c000;

static void *memcpy(void *dest, const void *src, u32 n)
{
	// Inspired by jgraef at osdev
	u32 num_dwords = n / 4;
	u32 num_bytes = n % 4;
	u32 *dest32 = (u32 *)dest;
	const u32 *src32 = (const u32 *)src;
	u8 *dest8 = ((u8 *)dest) + num_dwords * 4;
	const u8 *src8 = ((const u8 *)src) + num_dwords * 4;

	__asm__ volatile("rep movsl\n"
			 : "=S"(src32), "=D"(dest32), "=c"(num_dwords)
			 : "S"(src32), "D"(dest32), "c"(num_dwords)
			 : "memory");

	for (u32 i = 0; i < num_bytes; i++)
		dest8[i] = src8[i];

	return dest;
}

static void *memset(void *dest, u32 val, u32 n)
{
	// Inspired by jgraef at osdev
	u32 uval = val;
	u32 num_dwords = n / 4;
	u32 num_bytes = n % 4;
	u32 *dest32 = (u32 *)dest;
	u8 *dest8 = ((u8 *)dest) + num_dwords * 4;
	u8 val8 = (u8)val;
	u32 val32 = uval | (uval << 8) | (uval << 16) | (uval << 24);

	__asm__ volatile("rep stosl\n"
			 : "=D"(dest32), "=c"(num_dwords)
			 : "D"(dest32), "c"(num_dwords), "a"(val32)
			 : "memory");

	for (u32 i = 0; i < num_bytes; i++)
		dest8[i] = val8;

	return dest;
}

static void *malloc(u32 size)
{
	return (u32 *)(heap += size);
}

static void *zalloc(u32 size)
{
	void *ret = malloc(size);
	memset(ret, 0, size);
	return ret;
}

static void free(void *ptr)
{
	(void)ptr;
}

/**
 * String
 */

static u32 strlen(const char *str)
{
	const char *s = str;
	while (*s)
		s++;
	return s - str;
}

static s32 strncmp(const char *s1, const char *s2, u32 n)
{
	const u8 *c1 = (const u8 *)s1;
	const u8 *c2 = (const u8 *)s2;
	u8 ch;
	int d = 0;

	while (n--) {
		d = (int)(ch = *c1++) - (int)*c2++;
		if (d || !ch)
			break;
	}

	return d;
}

static char *strdup(const char *s)
{
	int l = strlen(s) + 1;
	char *d = malloc(l);

	memcpy(d, s, l);

	return d;
}

/**
 * CPU IO
 */

static u8 inb(u16 port)
{
	u8 value;
	__asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
	return value;
}

static u16 inw(u16 port)
{
	u16 value;
	__asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
	return value;
}

static void outb(u16 port, u8 data)
{
	__asm__ volatile("outb %0, %1" ::"a"(data), "Nd"(port));
}

/**
 * Serial
 */

static void serial_install(void)
{
	outb(0x3f8 + 1, 0x00);
	outb(0x3f8 + 3, 0x80);
	outb(0x3f8 + 0, 0x03);
	outb(0x3f8 + 1, 0x00);
	outb(0x3f8 + 3, 0x03);
	outb(0x3f8 + 2, 0xC7);
	outb(0x3f8 + 4, 0x0B);
}

static int is_transmit_empty(void)
{
	return inb(0x3f8 + 5) & 0x20;
}

static void serial_put(char ch)
{
	while (is_transmit_empty() == 0)
		;
	outb(0x3f8, (u8)ch);
}

static void print(const char *data)
{
	for (u32 i = 0; i < strlen(data); i++)
		serial_put(data[i]);
}

/**
 * IDE/ATA
 */

static u8 *ide_buf = NULL;

static void ide_select_drive(u8 bus, u8 drive)
{
	if (bus == ATA_PRIMARY) {
		if (drive == ATA_MASTER)
			outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xa0);
		else
			outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xb0);
	} else {
		if (drive == ATA_MASTER)
			outb(ATA_SECONDARY_IO + ATA_REG_HDDEVSEL, 0xa0);
		else
			outb(ATA_SECONDARY_IO + ATA_REG_HDDEVSEL, 0xb0);
	}
}

static u8 ide_find(u8 bus, u8 drive)
{
	u16 io = bus == ATA_PRIMARY ? ATA_PRIMARY_IO : ATA_SECONDARY_IO;
	ide_select_drive(bus, drive);

	// Reset
	outb(io + ATA_REG_SECCOUNT0, 0);
	outb(io + ATA_REG_LBA0, 0);
	outb(io + ATA_REG_LBA1, 0);
	outb(io + ATA_REG_LBA2, 0);

	// Identify
	outb(io + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
	u8 status = inb(io + ATA_REG_STATUS);
	if (!status)
		return 0;

	while ((inb(io + ATA_REG_STATUS) & ATA_SR_BSY) != 0)
		;

	do {
		status = inb(io + ATA_REG_STATUS);
		if (status & ATA_SR_ERR)
			return 0;
	} while ((status & ATA_SR_DRQ) == 0);

	for (int i = 0; i < BLOCK_COUNT; i++)
		*(u16 *)(ide_buf + i * 2) = inw(io + ATA_REG_DATA);

	return 1;
}

static void ide_delay(u16 io) // 400ns
{
	for (int i = 0; i < 4; i++)
		inb(io + ATA_REG_ALTSTATUS);
}

static void ide_poll(u16 io)
{
	for (int i = 0; i < 4; i++)
		inb(io + ATA_REG_ALTSTATUS);

	u8 status;
	do {
		status = inb(io + ATA_REG_STATUS);
	} while (status & ATA_SR_BSY);

	do {
		status = inb(io + ATA_REG_STATUS);
		/* assert(!(status & ATA_SR_ERR)) */
	} while (!(status & ATA_SR_DRQ));
}

static u8 ata_read_one(u8 *buf, u32 lba, u8 drive)
{
	u16 io = (drive & ATA_PRIMARY << 1) == ATA_PRIMARY ? ATA_PRIMARY_IO : ATA_SECONDARY_IO;
	drive = (drive & ATA_SLAVE) == ATA_SLAVE ? ATA_SLAVE : ATA_MASTER;
	u8 cmd = drive == ATA_MASTER ? 0xe0 : 0xf0;
	outb(io + ATA_REG_HDDEVSEL, (cmd | (u8)((lba >> 24 & 0x0f))));
	outb(io + 1, 0x00);
	outb(io + ATA_REG_SECCOUNT0, 1);
	outb(io + ATA_REG_LBA0, (u8)lba);
	outb(io + ATA_REG_LBA1, (u8)(lba >> 8));
	outb(io + ATA_REG_LBA2, (u8)(lba >> 16));
	outb(io + ATA_REG_COMMAND, ATA_CMD_READ_PIO);
	ide_poll(io);

	for (int i = 0; i < BLOCK_COUNT; i++) {
		u16 data = inw(io + ATA_REG_DATA);
		*(u16 *)(buf + i * 2) = data;
	}
	ide_delay(io);
	return 1;
}

static u32 ata_read(void *buf, u32 lba, u32 sector_count, u8 drive)
{
	u8 *b = buf; // I love bytes, yk
	for (u32 i = 0; i < sector_count; i++) {
		ata_read_one(b, lba + i, drive);
		b += SECTOR_SIZE;
	}
	return sector_count;
}

static u8 ata_probe(void)
{
	for (u8 i = 0; i < 4; i++) {
		u32 bus = i < 2 ? ATA_PRIMARY : ATA_SECONDARY;
		u32 drive = i % 2 ? ATA_MASTER : ATA_SLAVE;

		if (!ide_find(bus, drive))
			continue;

		u8 found_drive = (bus << 1) | drive;
		// TODO: What about the other drives?
		return found_drive;
	}

	return 0;
}

/**
 * EXT2
 */

static void *buffer_read(u32 block, u8 drive)
{
	void *buf = zalloc(BLOCK_SIZE);
	ata_read(buf, block * SECTOR_COUNT, SECTOR_COUNT, drive);
	return buf;
}

static struct ext2_superblock *get_superblock(u8 drive)
{
	struct ext2_superblock *sb = buffer_read(EXT2_SUPER, drive);

	assert(sb->magic == EXT2_MAGIC);
	return sb;
}

static struct ext2_bgd *get_bgd(u8 drive)
{
	return buffer_read(EXT2_SUPER + 1, drive);
}

static struct ext2_inode *get_inode(u32 i, struct ext2_inode *in_buf, u8 drive)
{
	struct ext2_superblock *s = get_superblock(drive);
	assert(s);
	struct ext2_bgd *b = get_bgd(drive);
	assert(b);

	u32 block_group = (i - 1) / s->inodes_per_group;
	u32 index = (i - 1) % s->inodes_per_group;
	u32 block = (index * EXT2_INODE_SIZE) / BLOCK_SIZE;
	b += block_group;

	u32 *buf = buffer_read(b->inode_table + block, drive);
	struct ext2_inode *in =
		(struct ext2_inode *)((u32)buf +
				      (index % (BLOCK_SIZE / EXT2_INODE_SIZE)) * EXT2_INODE_SIZE);

	memcpy(in_buf, in, sizeof(*in_buf));
	free(buf);
	free(s);
	free(b - block_group);

	return in_buf;
}

static u32 find_inode(const char *name, u32 dir_inode, u8 drive)
{
	if (!dir_inode)
		return (unsigned)-1;

	struct ext2_inode i = { 0 };
	get_inode(dir_inode, &i, drive);

	char *buf = malloc(BLOCK_SIZE * i.blocks / 2);
	memset(buf, 0, BLOCK_SIZE * i.blocks / 2);

	for (u32 q = 0; q < i.blocks / 2; q++) {
		char *data = buffer_read(i.block[q], drive);
		memcpy((u32 *)((u32)buf + q * BLOCK_SIZE), data, BLOCK_SIZE);
		free(data);
	}

	struct ext2_dirent *d = (struct ext2_dirent *)buf;

	u32 sum = 0;
	do {
		// Calculate the 4byte aligned size of each entry
		sum += d->total_len;
		if (strlen(name) == d->name_len &&
		    strncmp((void *)d->name, name, d->name_len) == 0) {
			free(buf);
			return d->inode_num;
		}
		d = (struct ext2_dirent *)((u32)d + d->total_len);

	} while (sum < (1024 * i.blocks / 2));
	free(buf);
	return (unsigned)-1;
}

static u32 read_indirect(u32 indirect, u32 block_num, u8 drive)
{
	void *data = buffer_read(indirect, drive);
	u32 ind = *(u32 *)((u32)data + block_num * sizeof(u32));
	free(data);
	return ind;
}

static s32 read_inode(struct ext2_inode *in, void *buf, u32 offset, u32 count, u8 drive)
{
	if (!in || !buf)
		return -1;

	if (in->size == 0)
		return 0;

	u32 num_blocks = in->blocks / (BLOCK_SIZE / SECTOR_SIZE) + 1;

	if (!num_blocks)
		return -1;

	u32 first_block = offset / BLOCK_SIZE;
	u32 last_block = (offset + count) / BLOCK_SIZE;
	if (last_block >= num_blocks)
		last_block = num_blocks - 1;
	u32 first_block_offset = offset % BLOCK_SIZE;

	u32 remaining = MIN(count, in->size - offset);
	u32 copied = 0;

	u32 indirect = 0;
	u32 blocknum = 0;

	// TODO: Support triply indirect pointers
	for (u32 i = first_block; i <= last_block; i++) {
		if (i < 12) {
			blocknum = in->block[i];
		} else if (i < BLOCK_COUNT + 12) {
			indirect = in->block[12];
			blocknum = read_indirect(indirect, i - 12, drive);
		} else {
			indirect = in->block[13];
			blocknum = read_indirect(indirect, (i - (BLOCK_COUNT + 12)) / BLOCK_COUNT,
						 drive);
			blocknum = read_indirect(blocknum, (i - (BLOCK_COUNT + 12)) % BLOCK_COUNT,
						 drive);
		}

		char *data = buffer_read(blocknum, drive);
		u32 block_offset = (i == first_block) ? first_block_offset : 0;
		u32 byte_count = MIN(BLOCK_SIZE - block_offset, remaining);

		memcpy((u8 *)buf + copied, data + block_offset, byte_count);

		copied += byte_count;
		remaining -= byte_count;

		free(data);
	}

	return copied;
}

static struct ext2_inode *find_inode_by_path(const char *path, struct ext2_inode *in_buf, u8 drive)
{
	char *path_cp = strdup(path);
	char *init = path_cp; // For freeing

	if (path_cp[0] != '/') {
		free(init);
		return NULL;
	}

	path_cp++;
	u32 current_inode = EXT2_ROOT;

	u32 i = 0;
	while (1) {
		for (i = 0; path_cp[i] != '/' && path_cp[i] != '\0'; i++)
			;

		if (path_cp[i] == '\0')
			break;

		path_cp[i] = '\0';
		current_inode = find_inode(path_cp, current_inode, drive);
		path_cp[i] = '/';

		if (current_inode == 0) {
			free(init);
			return NULL;
		}

		path_cp += i + 1;
	}

	u32 inode = find_inode(path_cp, current_inode, drive);
	free(init);
	if ((signed)inode <= 0)
		return NULL;

	return get_inode(inode, in_buf, drive);
}

static s32 read(const char *path, void *buf, u32 offset, u32 count, u8 drive)
{
	struct ext2_inode in = { 0 };
	if (find_inode_by_path(path, &in, drive) == &in) {
		return read_inode(&in, buf, offset, count, drive);
	} else {
		print("Couldn't find kernel!\n");
		return -1;
	}
}

/**
 * ELF
 */

static s32 elf_load(const char *path, u8 drive)
{
	struct elf_header header = { 0 };
	s32 rd = read(path, &header, 0, sizeof(header), drive);
	if (rd < 0)
		return rd;
	if (rd != sizeof(header))
		return -1;

	// Valid?
	u8 *magic = header.ident;
	u8 valid_magic = magic[ELF_IDENT_MAG0] == ELF_MAG0 && magic[ELF_IDENT_MAG1] == ELF_MAG1 &&
			 magic[ELF_IDENT_MAG2] == ELF_MAG2 && magic[ELF_IDENT_MAG3] == ELF_MAG3 &&
			 magic[ELF_IDENT_CLASS] == ELF_IDENT_CLASS_32 &&
			 magic[ELF_IDENT_DATA] == ELF_IDENT_DATA_LSB;
	if (!valid_magic || (header.type != ELF_ETYPE_REL && header.type != ELF_ETYPE_EXEC) ||
	    header.version != 1 || header.machine != ELF_MACHINE_386)
		return -1;

	// Loop through programs
	for (u32 i = 0; i < header.phnum; i++) {
		struct elf_program program = { 0 };

		if (read(path, &program, header.phoff + header.phentsize * i, sizeof(program),
			 drive) != sizeof(program))
			return -1;

		if (program.type == ELF_PROGRAM_TYPE_INTERP)
			return -1;

		if (program.vaddr == 0 || program.type != ELF_PROGRAM_TYPE_LOAD)
			continue;

		if ((u32)read(path, (void *)program.vaddr, program.offset, program.filesz, drive) !=
		    program.filesz)
			return -1;
	}

	// Find section string table
	struct elf_section section_strings = { 0 };
	if (read(path, &section_strings, header.shoff + header.shentsize * header.shstrndx,
		 sizeof(section_strings), drive) != sizeof(section_strings))
		return -1;

	if (section_strings.type != ELF_SECTION_TYPE_STRTAB)
		return -1;

	return header.entry;
}

/**
 * Let's go!
 */

int main(void *first, void *second)
{
	serial_install();
	print("Loaded bootloader!\n");

	u8 drive = ata_probe();
	assert(drive);

	s32 elf = elf_load("/apps/kernel/exec", drive);
	assert(elf > 0);

	void (*kernel)(void *, void *);
	*(void **)(&kernel) = (void *)elf;

	print("Loaded kernel!\n");
	kernel(first, second);

	print("WTF, kernel returned!\n");

	while (1)
		;

	return 0;
}

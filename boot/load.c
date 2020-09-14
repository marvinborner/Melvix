// MIT License, Copyright (c) 2020 Marvin Borner
// Independent ext2 loader

typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed long s32;
typedef unsigned long u32;

typedef signed long long s64;
typedef unsigned long long u64;

#define BLOCK_SIZE 1024
#define BLOCK_COUNT 256 // BLOCK_SIZE / sizeof(u32)
#define SECTOR_SIZE 512
#define IDE_BUSY (1 << 7)
#define IDE_READY (1 << 6)
#define IDE_DRIVE_FAULT (1 << 5)
#define IDE_ERROR (1 << 0)
#define IDE_IO 0x1F0
#define IDE_DATA 0x0
#define IDE_FEATURES 0x1
#define IDE_SECTOR_COUNT 0x2
#define IDE_LOW 0x3
#define IDE_MID 0x4
#define IDE_HIGH 0x5
#define IDE_HEAD 0x6
#define IDE_CMD 0x7
#define IDE_ALTERNATE 0x3F6
#define LBA_LOW(c) ((u8)(c & 0xFF))
#define LBA_MID(c) ((u8)(c >> 8) & 0xFF)
#define LBA_HIGH(c) ((u8)(c >> 16) & 0xFF)
#define LBA_LAST(c) ((u8)(c >> 24) & 0xF)
#define IDE_CMD_READ (BLOCK_SIZE / SECTOR_SIZE == 1) ? 0x20 : 0xC4
#define IDE_CMD_WRITE (BLOCK_SIZE / SECTOR_SIZE == 1) ? 0x30 : 0xC5
#define IDE_CMD_READ_MUL 0xC4
#define IDE_CMD_WRITE_MUL 0xC5

#define EXT2_BOOT 0
#define EXT2_SUPER 1
#define EXT2_ROOT 2
#define EXT2_MAGIC 0x0000EF53

struct superblock {
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

struct bgd {
	u32 block_bitmap;
	u32 inode_bitmap;
	u32 inode_table;
	u16 free_blocks;
	u16 free_inodes;
	u16 used_dirs;
	u16 pad;
	u8 bg_reserved[12];
};

struct inode {
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

#define INODE_SIZE (sizeof(struct inode))

struct dirent {
	u32 inode_num;
	u16 total_len;
	u8 name_len;
	u8 type_indicator;
	u8 name[];
};

struct file {
	struct inode inode;
	u32 pos;
	u8 block_index;
	u8 *buf;
	u32 curr_block_pos;
};

static u32 heap;
void *read_inode(struct inode *in);
struct inode *get_inode(int i);
int find_inode(const char *name, int dir_inode);
void serial_install();
void serial_print(const char *data);

int main(void *data)
{
	serial_install();
	heap = 0xf000;
	void (*entry)();
	*(void **)(&entry) = read_inode(get_inode(find_inode("kernel.bin", 2)));
	if (entry) {
		serial_print("Loaded kernel!\n");
		entry(data);
	} else {
		serial_print("Couldn't find kernel!\n");
	}
	while (1) {
	};
	return 0;
}

u8 inb(u16 port)
{
	u8 value;
	__asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
	return value;
}

void insl(u16 port, void *addr, int n)
{
	__asm__ volatile("cld; rep insl"
			 : "=D"(addr), "=c"(n)
			 : "d"(port), "0"(addr), "1"(n)
			 : "memory", "cc");
}

void outb(u16 port, u8 data)
{
	__asm__ volatile("outb %0, %1" ::"a"(data), "Nd"(port));
}

void *memcpy(void *dst, const void *src, u32 n)
{
	const char *sp = (const char *)src;
	char *dp = (char *)dst;
	for (; n != 0; n--)
		*dp++ = *sp++;
	return dst;
}

void *memset(void *dst, char val, u32 n)
{
	char *temp = (char *)dst;
	for (; n != 0; n--)
		*temp++ = val;
	return dst;
}

int strncmp(const char *s1, const char *s2, u32 n)
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

u32 strlen(const char *s)
{
	const char *ss = s;
	while (*ss)
		ss++;
	return ss - s;
}

void serial_install()
{
	outb(0x3f8 + 1, 0x00);
	outb(0x3f8 + 3, 0x80);
	outb(0x3f8 + 0, 0x03);
	outb(0x3f8 + 1, 0x00);
	outb(0x3f8 + 3, 0x03);
	outb(0x3f8 + 2, 0xC7);
	outb(0x3f8 + 4, 0x0B);
}

int is_transmit_empty()
{
	return inb(0x3f8 + 5) & 0x20;
}

void serial_put(char ch)
{
	while (is_transmit_empty() == 0)
		;
	outb(0x3f8, (u8)ch);
}

void serial_print(const char *data)
{
	for (u32 i = 0; i < strlen(data); i++)
		serial_put(data[i]);
}

void *malloc(u32 size)
{
	return (u32 *)(heap += size);
}

int ide_wait(int check)
{
	char r;

	// Wait while drive is busy. Once just ready is set, exit the loop
	while (((r = (char)inb(IDE_IO | IDE_CMD)) & (IDE_BUSY | IDE_READY)) != IDE_READY)
		;

	// Check for errors
	if (check && (r & (IDE_DRIVE_FAULT | IDE_ERROR)) != 0)
		return 0xF;
	return 0;
}

void *ide_read(void *b, u32 block)
{
	int sector_per_block = BLOCK_SIZE / SECTOR_SIZE; // 2
	int sector = block * sector_per_block;

	ide_wait(0);
	outb(IDE_IO | IDE_SECTOR_COUNT, sector_per_block); // Number of sectors
	outb(IDE_IO | IDE_LOW, LBA_LOW(sector));
	outb(IDE_IO | IDE_MID, LBA_MID(sector));
	outb(IDE_IO | IDE_HIGH, LBA_HIGH(sector));

	// Slave/Master << 4 and last 4 bits
	outb(IDE_IO | IDE_HEAD, 0xE0 | (1 << 4) | LBA_LAST(sector));
	outb(IDE_IO | IDE_CMD, IDE_CMD_READ);
	ide_wait(0);

	// Read-only
	insl(IDE_IO, b, BLOCK_SIZE / 4);

	return b;
}

void *buffer_read(int block)
{
	return ide_read(malloc(BLOCK_SIZE), block);
}

struct superblock *get_superblock()
{
	struct superblock *sb = buffer_read(EXT2_SUPER);
	if (sb->magic != EXT2_MAGIC)
		return 0;
	return sb;
}

struct bgd *get_bgd()
{
	return buffer_read(EXT2_SUPER + 1);
}

struct inode *get_inode(int i)
{
	struct superblock *s = get_superblock();
	//assert(s);
	struct bgd *b = get_bgd();
	//assert(b);

	int block_group = (i - 1) / s->inodes_per_group;
	int index = (i - 1) % s->inodes_per_group;
	int block = (index * INODE_SIZE) / BLOCK_SIZE;
	b += block_group;

	u32 *data = buffer_read(b->inode_table + block);
	struct inode *in =
		(struct inode *)((u32)data + (index % (BLOCK_SIZE / INODE_SIZE)) * INODE_SIZE);
	return in;
}

u32 read_indirect(u32 indirect, u32 block_num)
{
	char *data = buffer_read(indirect);
	return *(u32 *)((u32)data + block_num * sizeof(u32));
}

void *read_inode(struct inode *in)
{
	//assert(in);
	if (!in)
		return 0;

	int num_blocks = in->blocks / (BLOCK_SIZE / SECTOR_SIZE);

	//assert(num_blocks != 0);
	if (!num_blocks)
		return 0;

	void *buf = (void *)0x50000;
	//assert(buf != 0);

	int indirect;

	int blocknum;
	char *data;
	for (int i = 0; i < num_blocks; i++) {
		if (i < 12) {
			blocknum = in->block[i];
			data = buffer_read(blocknum);
			memcpy((u32 *)((u32)buf + i * BLOCK_SIZE), data, BLOCK_SIZE);
		} else if (i < BLOCK_COUNT + 12) {
			indirect = in->block[12];
			blocknum = read_indirect(indirect, i - 12);
			data = buffer_read(blocknum);
			memcpy((u32 *)((u32)buf + i * BLOCK_SIZE), data, BLOCK_SIZE);
		} else {
			indirect = in->block[13];
			blocknum = read_indirect(indirect, (i - (BLOCK_COUNT + 12)) / BLOCK_COUNT);
			blocknum = read_indirect(blocknum, (i - (BLOCK_COUNT + 12)) % BLOCK_COUNT);
			data = buffer_read(blocknum);
			memcpy((u32 *)((u32)buf + i * BLOCK_SIZE), data, BLOCK_SIZE);
		}
	}

	return buf;
}

int find_inode(const char *name, int dir_inode)
{
	if (!dir_inode)
		return -1;

	struct inode *i = get_inode(dir_inode);

	char *buf = malloc(BLOCK_SIZE * i->blocks / 2);
	memset(buf, 0, BLOCK_SIZE * i->blocks / 2);

	for (u32 q = 0; q < i->blocks / 2; q++) {
		char *data = buffer_read(i->block[q]);
		memcpy((u32 *)((u32)buf + q * BLOCK_SIZE), data, BLOCK_SIZE);
	}

	struct dirent *d = (struct dirent *)buf;

	u32 sum = 0;
	do {
		// Calculate the 4byte aligned size of each entry
		sum += d->total_len;
		if (strncmp((void *)d->name, name, d->name_len) == 0) {
			return d->inode_num;
		}
		d = (struct dirent *)((u32)d + d->total_len);

	} while (sum < (1024 * i->blocks / 2));
	return -1;
}

// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <def.h>
#include <mem.h>
#include <sys.h>

void *memcpy(void *dest, const void *src, u32 n)
{
#ifdef userspace
	// Inspired by Jeko at osdev
	u8 *dest_byte = dest;
	const u8 *src_byte = src;
	for (u32 i = 0; i < n / 16; i++) {
		__asm__ volatile("movups (%0), %%xmm0\n"
				 "movntdq %%xmm0, (%1)\n" ::"r"(src_byte),
				 "r"(dest_byte)
				 : "memory");

		src_byte += 16;
		dest_byte += 16;
	}

	if (n & 7) {
		n = n & 7;

		int d0, d1, d2;
		__asm__ volatile("rep ; movsl\n\t"
				 "testb $2,%b4\n\t"
				 "je 1f\n\t"
				 "movsw\n"
				 "1:\ttestb $1,%b4\n\t"
				 "je 2f\n\t"
				 "movsb\n"
				 "2:"
				 : "=&c"(d0), "=&D"(d1), "=&S"(d2)
				 : "0"(n / 4), "q"(n), "1"((long)dest_byte), "2"((long)src_byte)
				 : "memory");
	}
	return dest_byte;
#else
	// Inspired by jgraef at osdev
	u32 num_dwords = n / 4;
	u32 num_bytes = n % 4;
	u32 *dest32 = (u32 *)dest;
	const u32 *src32 = (const u32 *)src;
	u8 *dest8 = ((u8 *)dest) + num_dwords * 4;
	const u8 *src8 = ((const u8 *)src) + num_dwords * 4;

	// TODO: What's faster?
	__asm__ volatile("rep movsl\n"
			 : "=S"(src32), "=D"(dest32), "=c"(num_dwords)
			 : "S"(src32), "D"(dest32), "c"(num_dwords)
			 : "memory");

	/* for (u32 i = 0; i < num_dwords; i++) { */
	/* 	dest32[i] = src32[i]; */
	/* } */

	for (u32 i = 0; i < num_bytes; i++) {
		dest8[i] = src8[i];
	}
	return dest;
#endif
}

void *memset(void *dest, int val, u32 n)
{
	u32 num_dwords = n / 4;
	u32 num_bytes = n % 4;
	u32 *dest32 = (u32 *)dest;
	u8 *dest8 = ((u8 *)dest) + num_dwords * 4;
	u8 val8 = (u8)val;
	u32 val32 = val | (val << 8) | (val << 16) | (val << 24);

	// TODO: What's faster?
	__asm__ volatile("rep stosl\n"
			 : "=D"(dest32), "=c"(num_dwords)
			 : "D"(dest32), "c"(num_dwords), "a"(val32)
			 : "memory");

	/* for (u32 i = 0; i < num_dwords; i++) { */
	/* 	dest32[i] = val32; */
	/* } */

	for (u32 i = 0; i < num_bytes; i++) {
		dest8[i] = val8;
	}
	return dest;
}

void *memchr(void *src, int c, u32 n)
{
	u8 *s = (u8 *)src;

	while (n-- > 0) {
		if (*s == c)
			return s;
		s++;
	}
	return NULL;
}

int memcmp(const void *s1, const void *s2, u32 n)
{
	const u8 *a = (const u8 *)s1;
	const u8 *b = (const u8 *)s2;
	for (u32 i = 0; i < n; i++) {
		if (a[i] < b[i])
			return -1;
		else if (b[i] < a[i])
			return 1;
	}
	return 0;
}

int mememp(const u8 *buf, u32 n)
{
	return buf[0] == 0 && !memcmp(buf, buf + 1, n - 1);
}

/**
 * Heap allocator
 * Inspired by SHMALL (MIT License)
 * Copyright (c) 2017 Chris Careaga
 * Copyright (c) 2021 Marvin Borner
 */

#ifdef kernel

#define HEAP_MAGIC 0x424242
#define HEAP_INIT_SIZE 0xf000000
#define HEAP_MIN_SIZE HEAP_INIT_SIZE
#define MIN_ALLOC_SZ 4
#define BIN_COUNT 9
#define BIN_MAX_IDX (BIN_COUNT - 1)
#define OVERHEAD (sizeof(struct h_footer) + sizeof(struct h_node))
/* #define MIN_WILDERNESS 0x2000 */
/* #define MAX_WILDERNESS 0x1000000 */

struct h_node {
	u32 magic;
	u32 hole;
	u32 size;
	struct h_node *next;
	struct h_node *prev;
};

struct h_footer {
	struct h_node *header;
};

struct h_bin {
	struct h_node *head;
};

struct heap {
	u32 start;
	u32 end;
	struct h_bin bins[BIN_COUNT];
};

static void node_add(struct h_bin *bin, struct h_node *node)
{
	node->magic = HEAP_MAGIC;
	node->next = NULL;
	node->prev = NULL;
	if (!bin->head) {
		bin->head = node;
		return;
	}
	struct h_node *curr = bin->head;
	struct h_node *prev = NULL;
	while (curr && curr->size <= node->size) {
		prev = curr;
		curr = curr->next;
	}
	if (!curr) {
		prev->next = node;
		node->prev = prev;
	} else if (prev) {
		node->next = curr;
		prev->next = node;
		node->prev = prev;
		curr->prev = node;
	} else {
		node->next = bin->head;
		bin->head->prev = node;
		bin->head = node;
	}
}

static void node_remove(struct h_bin *bin, struct h_node *node)
{
	if (!bin->head)
		return;
	if (bin->head == node) {
		bin->head = bin->head->next;
		return;
	}

	struct h_node *temp = bin->head->next;
	while (temp) {
		if (temp == node) {
			if (!temp->next) {
				temp->prev->next = NULL;
			} else {
				temp->prev->next = temp->next;
				temp->next->prev = temp->prev;
			}
			return;
		}
		temp = temp->next;
	}
}

static struct h_node *node_best_fit(struct h_bin *bin, u32 size)
{
	if (!bin->head)
		return NULL;

	struct h_node *temp = bin->head;
	while (temp) {
		if (temp->size >= size)
			return temp;
		temp = temp->next;
	}
	return NULL;
}

/* static struct h_node *node_last(struct h_bin *bin) */
/* { */
/* 	struct h_node *temp = bin->head; */
/* 	while (temp->next) */
/* 		temp = temp->next; */
/* 	return temp; */
/* } */

static struct h_footer *node_foot(struct h_node *node)
{
	return (struct h_footer *)((char *)node + sizeof(*node) + node->size);
}

static void node_create_foot(struct h_node *head)
{
	struct h_footer *foot = node_foot(head);
	foot->header = head;
}

static u32 bin_index(u32 sz)
{
	u32 index = 0;
	sz = sz < 4 ? 4 : sz;
	while (sz >>= 1)
		index++;
	index -= 2;
	if (index > BIN_MAX_IDX)
		index = BIN_MAX_IDX;
	return index;
}

/* struct h_node *wilderness_get(struct heap *heap) */
/* { */
/* 	struct h_footer *wild_foot = (struct h_footer *)((char *)heap->end - sizeof(*wild_foot)); */
/* 	return wild_foot->header; */
/* } */

/* static u32 expand(struct heap *heap, u32 sz) */
/* { */
/* 	(void)heap; */
/* 	(void)sz; */
/* 	return 0; */
/* } */

/* static u32 contract(struct heap *heap, u32 sz) */
/* { */
/* 	(void)heap; */
/* 	(void)sz; */
/* 	return 0; */
/* } */

static struct heap heap = { 0 };
void heap_init(u32 start)
{
	struct h_node *init_region = (struct h_node *)start;
	init_region->hole = 1;
	init_region->size = HEAP_INIT_SIZE - OVERHEAD;
	node_create_foot(init_region);
	node_add(&heap.bins[bin_index(init_region->size)], init_region);
	heap.start = (u32)start;
	heap.end = (u32)start + HEAP_INIT_SIZE;
}

#define ALIGN sizeof(long)
static void *_malloc(u32 size)
{
	size = ((size + ALIGN - 1) / ALIGN) * ALIGN; // Alignment
	u32 index = bin_index(size);
	struct h_bin *temp = (struct h_bin *)&heap.bins[index];
	struct h_node *found = node_best_fit(temp, size);

	while (!found) {
		assert(index + 1 < BIN_COUNT);

		temp = &heap.bins[++index];
		found = node_best_fit(temp, size);
	}

	assert(found->magic == HEAP_MAGIC);

	if ((found->size - size) > (OVERHEAD + MIN_ALLOC_SZ)) {
		struct h_node *split = (struct h_node *)(((char *)found + OVERHEAD) + size);
		split->magic = HEAP_MAGIC;
		split->size = found->size - size - OVERHEAD;
		split->hole = 1;

		node_create_foot(split);

		u32 new_idx = bin_index(split->size);

		node_add(&heap.bins[new_idx], split);

		found->size = size;
		node_create_foot(found);
	}

	found->hole = 0;
	node_remove(&heap.bins[index], found);

	// TODO: Implement expand/contract
	/* struct h_node *wild = wilderness_get(&heap); */
	/* if (wild->size < MIN_WILDERNESS) { */
	/* 	assert(expand(&heap, 0x1000)); */
	/* } else if (wild->size > MAX_WILDERNESS) { */
	/* 	assert(contract(&heap, 0x1000)); */
	/* } */

	found->prev = NULL;
	found->next = NULL;
	return &found->next;
}

static void _free(void *p)
{
	if (!p)
		return;

	struct h_bin *list;
	struct h_footer *new_foot, *old_foot;

	struct h_node *head = (struct h_node *)((char *)p - 12);
	assert(head->magic == HEAP_MAGIC && head->hole == 0);
	if (head == (struct h_node *)(u32 *)heap.start) {
		head->hole = 1;
		node_add(&heap.bins[bin_index(head->size)], head);
		return;
	}

	struct h_node *next = (struct h_node *)((char *)node_foot(head) + sizeof(struct h_footer));
	struct h_footer *f = (struct h_footer *)((char *)head - sizeof(struct h_footer));
	struct h_node *prev = f->header;

	if (prev->hole) {
		list = &heap.bins[bin_index(prev->size)];
		node_remove(list, prev);

		prev->size += OVERHEAD + head->size;
		new_foot = node_foot(head);
		new_foot->header = prev;

		head = prev;
	}

	if (next->hole) {
		list = &heap.bins[bin_index(next->size)];
		node_remove(list, next);

		head->size += OVERHEAD + next->size;

		old_foot = node_foot(next);
		old_foot->header = 0;
		next->size = 0;
		next->hole = 0;

		new_foot = node_foot(head);
		new_foot->header = head;
	}

	head->hole = 1;
	node_add(&heap.bins[bin_index(head->size)], head);
}

#elif defined(userspace)

#define kmalloc(n) (void *)sys1(SYS_MALLOC, n)
#define kfree(ptr) (void)(sys1(SYS_FREE, (int)ptr))

static void *_malloc(u32 size)
{
	return kmalloc(size);
}

static void _free(void *ptr)
{
	kfree(ptr);
}

#endif

#ifdef kernel
#define PREFIX "K"
#define FUNC printf
#else
#define PREFIX "U"
#define FUNC log
#endif

void *zalloc(u32 size)
{
	void *ret = malloc(size);
	memset(ret, 0, size);
	return ret;
}

// Naive realloc implementation - TODO!
void *realloc(void *ptr, u32 size)
{
	if (!ptr)
		return malloc(size);

	FUNC("Realloc not implemented!\n");
	return NULL;
	/* // This could work; untested
	struct h_node *node = (struct h_node *)((char *)ptr - 12);
	u32 old_size = node->size;

	void *new = malloc(size);
	memcpy(new, ptr, old_size);

	free(ptr);
	return new;
	*/
}

void *malloc_debug(u32 size, const char *file, int line, const char *func, const char *inp)
{
	assert(size < (100 << 20)); // Don't brag with memory pls
	void *ret = _malloc(size);

	(void)file;
	(void)line;
	(void)func;
	(void)inp;
	/* FUNC(PREFIX "MALLOC\t%s:%d: %s: 0x%x %dB (%s)\n", file, line, func, ret, size, inp); */
	return ret;
}

void free_debug(void *ptr, const char *file, int line, const char *func, const char *inp)
{
	if (ptr)
		_free(ptr);

	(void)file;
	(void)line;
	(void)func;
	(void)inp;
	/* FUNC(PREFIX "FREE\t%s:%d: %s: 0x%x (%s)\n", file, line, func, ptr, inp); */
}

// MIT License, Copyright (c) 2021 Marvin Borner
// Mostly by Durand Miller, released into public domain

#include <assert.h>
#include <cpu.h>
#include <mem.h>

#ifdef kernel

#include <mm.h>

static void *liballoc_alloc(u32 p)
{
	return memory_alloc(virtual_kernel_dir(), p, MEMORY_CLEAR);
}

static int liballoc_free(void *ptr, u32 p)
{
	memory_free(virtual_kernel_dir(), memory_range((u32)ptr, (u32)p));
	return 0;
}

#else

#include <sys.h>

#define sys_alloc(size) (void *)sys1(SYS_ALLOC, size)
#define sys_free(ptr, size) (u32) sys2(SYS_FREE, ptr, size)

static void *liballoc_alloc(u32 p)
{
	return sys_alloc((u32)p);
}

static int liballoc_free(void *ptr, u32 p)
{
	sys_free((u32)ptr, (u32)p);
	return 0;
}

#endif

static int locked = 0;

static int liballoc_lock(void)
{
	spinlock(&locked);
	return 0;
}

static int liballoc_unlock(void)
{
	locked = 0;
	return 0;
}

#define ALIGNMENT 16
#define ALIGN_UP(__addr, __align) (((__addr) + (__align)-1) & ~((__align)-1))
#define ALIGN_DOWN(__addr, __align) ((__addr) & ~((__align)-1))

#define USE_CASE1
#define USE_CASE2
#define USE_CASE3
#define USE_CASE4
#define USE_CASE5
#define LIBALLOC_MAGIC 0x900df00d
#define LIBALLOC_DEAD 0xbaadf00d

struct liballoc_major {
	struct liballoc_major *prev;
	struct liballoc_major *next;
	u32 pages;
	u32 size;
	u32 usage;
	struct liballoc_minor *first;
};

struct liballoc_minor {
	struct liballoc_minor *prev;
	struct liballoc_minor *next;
	struct liballoc_major *block;
	u32 magic;
	u32 size;
	u32 req_size;
};

#define MAJOR_SIZE (ALIGN_UP(sizeof(struct liballoc_major), 16))
#define MINOR_SIZE (ALIGN_UP(sizeof(struct liballoc_minor), 16))

static struct liballoc_major *l_mem_root = NULL;
static struct liballoc_major *l_best_bet = NULL;

static u32 l_page_size = 4096;
static u32 l_page_count = 16;

static struct liballoc_major *allocate_new_page(u32 size)
{
	u32 st = size + MAJOR_SIZE + MINOR_SIZE;

	if ((st % l_page_size) == 0)
		st = st / (l_page_size);
	else
		st = st / (l_page_size) + 1;

	st = MAX(st, l_page_count);

	struct liballoc_major *maj = (struct liballoc_major *)liballoc_alloc(st * l_page_size);

	if (maj == NULL)
		return NULL;

	maj->prev = NULL;
	maj->next = NULL;
	maj->pages = st;
	maj->size = st * l_page_size;
	maj->usage = MAJOR_SIZE;
	maj->first = NULL;

	return maj;
}

static void *_malloc(u32 req_size)
{
	req_size = ALIGN_UP(req_size, 16);

	u32 best_size = 0;
	u32 size = req_size;

	liballoc_lock();

	if (size == 0) {
		liballoc_unlock();
		return malloc(1);
	}

	if (l_mem_root == NULL) {
		l_mem_root = allocate_new_page(size);
		if (l_mem_root == NULL) {
			liballoc_unlock();
			panic("Malloc failed!\n");
		}
	}

	struct liballoc_major *maj = l_mem_root;
	u8 started_bet = 0;

	if (l_best_bet != NULL) {
		best_size = l_best_bet->size - l_best_bet->usage;

		if (best_size > (size + MINOR_SIZE)) {
			maj = l_best_bet;
			started_bet = 1;
		}
	}

	while (maj != NULL) {
		u32 diff = maj->size - maj->usage;
		if (best_size < diff) {
			l_best_bet = maj;
			best_size = diff;
		}

#ifdef USE_CASE1
		if (diff < (size + MINOR_SIZE)) {
			if (maj->next != NULL) {
				maj = maj->next;
				continue;
			}

			if (started_bet == 1) {
				maj = l_mem_root;
				started_bet = 0;
				continue;
			}

			maj->next = allocate_new_page(size);
			if (maj->next == NULL)
				break;
			maj->next->prev = maj;
			maj = maj->next;
		}
#endif

#ifdef USE_CASE2
		if (maj->first == NULL) {
			maj->first = (struct liballoc_minor *)((u32)maj + MAJOR_SIZE);

			maj->first->magic = LIBALLOC_MAGIC;
			maj->first->prev = NULL;
			maj->first->next = NULL;
			maj->first->block = maj;
			maj->first->size = size;
			maj->first->req_size = req_size;
			maj->usage += size + MINOR_SIZE;
			void *p = (void *)((u32)(maj->first) + MINOR_SIZE);
			liballoc_unlock();
			return p;
		}
#endif

#ifdef USE_CASE3
		diff = (u32)(maj->first);
		diff -= (u32)maj;
		diff -= MAJOR_SIZE;

		if (diff >= (size + MINOR_SIZE)) {
			maj->first->prev = (struct liballoc_minor *)((u32)maj + MAJOR_SIZE);
			maj->first->prev->next = maj->first;
			maj->first = maj->first->prev;
			maj->first->magic = LIBALLOC_MAGIC;
			maj->first->prev = NULL;
			maj->first->block = maj;
			maj->first->size = size;
			maj->first->req_size = req_size;
			maj->usage += size + MINOR_SIZE;
			void *p = (void *)((u32)(maj->first) + MINOR_SIZE);
			liballoc_unlock();
			return p;
		}
#endif

#ifdef USE_CASE4
		struct liballoc_minor *min = maj->first;

		while (min != NULL) {
			if (min->next == NULL) {
				diff = (u32)(maj) + maj->size;
				diff -= (u32)min;
				diff -= MINOR_SIZE;
				diff -= min->size;
				if (diff >= (size + MINOR_SIZE)) {
					min->next =
						(struct liballoc_minor *)((u32)min + MINOR_SIZE +
									  min->size);
					min->next->prev = min;
					min = min->next;
					min->next = NULL;
					min->magic = LIBALLOC_MAGIC;
					min->block = maj;
					min->size = size;
					min->req_size = req_size;
					maj->usage += size + MINOR_SIZE;
					void *p = (void *)((u32)min + MINOR_SIZE);
					liballoc_unlock();
					return p;
				}
			}

			if (min->next != NULL) {
				diff = (u32)(min->next);
				diff -= (u32)min;
				diff -= MINOR_SIZE;
				diff -= min->size;

				if (diff >= (size + MINOR_SIZE)) {
					struct liballoc_minor *new_min =
						(struct liballoc_minor *)((u32)min + MINOR_SIZE +
									  min->size);
					new_min->magic = LIBALLOC_MAGIC;
					new_min->next = min->next;
					new_min->prev = min;
					new_min->size = size;
					new_min->req_size = req_size;
					new_min->block = maj;
					min->next->prev = new_min;
					min->next = new_min;
					maj->usage += size + MINOR_SIZE;
					void *p = (void *)((u32)new_min + MINOR_SIZE);
					liballoc_unlock();
					return p;
				}
			}

			min = min->next;
		}
#endif

#ifdef USE_CASE5
		if (maj->next == NULL) {
			if (started_bet == 1) {
				maj = l_mem_root;
				started_bet = 0;
				continue;
			}
			maj->next = allocate_new_page(size);
			if (maj->next == NULL)
				break;
			maj->next->prev = maj;
		}
#endif
		maj = maj->next;
	}

	liballoc_unlock();

	panic("Malloc failed!\n");
	return NULL;
}

static void _free(void *ptr)
{
	if (ptr == NULL) {
		return;
	}

	liballoc_lock();

	struct liballoc_minor *min = (struct liballoc_minor *)((u32)ptr - MINOR_SIZE);

	if (min->magic != LIBALLOC_MAGIC) {
		liballoc_unlock();
		return;
	}

	struct liballoc_major *maj = min->block;
	maj->usage -= (min->size + MINOR_SIZE);
	min->magic = LIBALLOC_DEAD;

	if (min->next != NULL)
		min->next->prev = min->prev;
	if (min->prev != NULL)
		min->prev->next = min->next;
	if (min->prev == NULL)
		maj->first = min->next;
	if (maj->first == NULL) {
		if (l_mem_root == maj)
			l_mem_root = maj->next;
		if (l_best_bet == maj)
			l_best_bet = NULL;
		if (maj->prev != NULL)
			maj->prev->next = maj->next;
		if (maj->next != NULL)
			maj->next->prev = maj->prev;
		liballoc_free(maj, maj->pages * l_page_size);
	} else {
		if (l_best_bet != NULL) {
			int best_size = l_best_bet->size - l_best_bet->usage;
			int maj_size = maj->size - maj->usage;
			if (maj_size > best_size)
				l_best_bet = maj;
		}
	}
	liballoc_unlock();
}

static void *_realloc(void *ptr, u32 size)
{
	size = ALIGN_UP(size, 16);

	if (size == 0) {
		free(ptr);
		return NULL;
	}

	if (ptr == NULL)
		return malloc(size);

	liballoc_lock();
	struct liballoc_minor *min = (struct liballoc_minor *)((u32)ptr - MINOR_SIZE);

	if (min->magic != LIBALLOC_MAGIC) {
		liballoc_unlock();
		panic("Malloc failed!\n");
		return NULL;
	}

	if (min->size >= size) {
		min->req_size = size;
		liballoc_unlock();
		return ptr;
	}

	liballoc_unlock();

	void *new_ptr = malloc(size);
	memcpy(new_ptr, ptr, min->req_size);
	free(ptr);

	return new_ptr;
}

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

void *realloc(void *ptr, u32 size)
{
	return _realloc(ptr, size);
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

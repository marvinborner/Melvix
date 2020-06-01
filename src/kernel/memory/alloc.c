#include <io/io.h>
#include <lib/lib.h>
#include <memory/alloc.h>
#include <memory/mmap.h>
#include <memory/paging.h>
#include <stddef.h>
#include <stdint.h>
#include <system.h>

static int locked = 0;

int liballoc_lock()
{
	spinlock(&locked);
	return 0;
}

int liballoc_unlock()
{
	locked = 0;
	return 0;
}

void *liballoc_alloc(u32 p)
{
	u32 ptr = kmalloc_frames((u32)p);
	return (void *)ptr;
}

int liballoc_free(void *ptr, u32 p)
{
	kfree_frames((u32)ptr, (u32)p);
	return 0;
}

#define ALIGNMENT 16ul
#define ALIGN_TYPE char
#define ALIGN_INFO sizeof(ALIGN_TYPE) * 16
#define USE_CASE1
#define USE_CASE2
#define USE_CASE3
#define USE_CASE4
#define USE_CASE5

#define ALIGN(ptr)                                                                                 \
	if (ALIGNMENT > 1) {                                                                       \
		u32 diff;                                                                          \
		ptr = (void *)((u32)ptr + ALIGN_INFO);                                             \
		diff = (u32)ptr & (ALIGNMENT - 1);                                                 \
		if (diff != 0) {                                                                   \
			diff = ALIGNMENT - diff;                                                   \
			ptr = (void *)((u32)ptr + diff);                                           \
		}                                                                                  \
		*((ALIGN_TYPE *)((u32)ptr - ALIGN_INFO)) = diff + ALIGN_INFO;                      \
	}

#define UNALIGN(ptr)                                                                               \
	if (ALIGNMENT > 1) {                                                                       \
		u32 diff = *((ALIGN_TYPE *)((u32)ptr - ALIGN_INFO));                               \
		if (diff < (ALIGNMENT + ALIGN_INFO)) {                                             \
			ptr = (void *)((u32)ptr - diff);                                           \
		}                                                                                  \
	}

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

static struct liballoc_major *l_mem_root = NULL;
static struct liballoc_major *l_best_bet = NULL;

static u32 l_page_size = 4096;
static u32 l_page_count = 16;
static u64 l_allocated = 0;
static u64 l_inuse = 0;

static long long l_warning_count = 0;
static long long l_error_count = 0;
static long long l_possible_overruns = 0;

static void *liballoc_memset(void *s, int c, u32 n)
{
	for (u32 i = 0; i < n; i++)
		((char *)s)[i] = c;

	return s;
}

static void *liballoc_memcpy(void *s1, const void *s2, u32 n)
{
	char *cdest;
	char *csrc;
	u32 *ldest = (u32 *)s1;
	u32 *lsrc = (u32 *)s2;

	while (n >= sizeof(u32)) {
		*ldest++ = *lsrc++;
		n -= sizeof(u32);
	}

	cdest = (char *)ldest;
	csrc = (char *)lsrc;

	while (n > 0) {
		*cdest++ = *csrc++;
		n -= 1;
	}
	return s1;
}

static struct liballoc_major *allocate_new_page(u32 size)
{
	u32 st;
	struct liballoc_major *maj;

	st = size + sizeof(struct liballoc_major);
	st += sizeof(struct liballoc_minor);

	if ((st % l_page_size) == 0)
		st = st / (l_page_size);
	else
		st = st / (l_page_size) + 1;

	if (st < l_page_count)
		st = l_page_count;

	maj = (struct liballoc_major *)liballoc_alloc(st);

	if (maj == NULL) {
		l_warning_count += 1;
		return NULL;
	}

	maj->prev = NULL;
	maj->next = NULL;
	maj->pages = st;
	maj->size = st * l_page_size;
	maj->usage = sizeof(struct liballoc_major);
	maj->first = NULL;

	l_allocated += maj->size;

	return maj;
}

void *malloc(u32 req_size)
{
	assert(paging_enabled);

	int started_bet = 0;
	u64 best_size = 0;
	void *p = NULL;
	u32 diff;
	struct liballoc_major *maj;
	struct liballoc_minor *min;
	struct liballoc_minor *new_min;
	u32 size = req_size;

	if (ALIGNMENT > 1) {
		size += ALIGNMENT + ALIGN_INFO;
	}

	liballoc_lock();

	if (size == 0) {
		l_warning_count += 1;
		liballoc_unlock();
		return malloc(1);
	}

	if (l_mem_root == NULL) {
		l_mem_root = allocate_new_page(size);
		if (l_mem_root == NULL) {
			liballoc_unlock();
			return NULL;
		}
	}

	maj = l_mem_root;
	started_bet = 0;

	if (l_best_bet != NULL) {
		best_size = l_best_bet->size - l_best_bet->usage;

		if (best_size > (size + sizeof(struct liballoc_minor))) {
			maj = l_best_bet;
			started_bet = 1;
		}
	}

	while (maj != NULL) {
		diff = maj->size - maj->usage;
		if (best_size < diff) {
			l_best_bet = maj;
			best_size = diff;
		}

#ifdef USE_CASE1
		if (diff < (size + sizeof(struct liballoc_minor))) {
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
			maj->first =
				(struct liballoc_minor *)((u32)maj + sizeof(struct liballoc_major));

			maj->first->magic = LIBALLOC_MAGIC;
			maj->first->prev = NULL;
			maj->first->next = NULL;
			maj->first->block = maj;
			maj->first->size = size;
			maj->first->req_size = req_size;
			maj->usage += size + sizeof(struct liballoc_minor);
			l_inuse += size;
			p = (void *)((u32)(maj->first) + sizeof(struct liballoc_minor));
			ALIGN(p);
			liballoc_unlock();
			return p;
		}
#endif

#ifdef USE_CASE3
		diff = (u32)(maj->first);
		diff -= (u32)maj;
		diff -= sizeof(struct liballoc_major);

		if (diff >= (size + sizeof(struct liballoc_minor))) {
			maj->first->prev =
				(struct liballoc_minor *)((u32)maj + sizeof(struct liballoc_major));
			maj->first->prev->next = maj->first;
			maj->first = maj->first->prev;
			maj->first->magic = LIBALLOC_MAGIC;
			maj->first->prev = NULL;
			maj->first->block = maj;
			maj->first->size = size;
			maj->first->req_size = req_size;
			maj->usage += size + sizeof(struct liballoc_minor);
			l_inuse += size;
			p = (void *)((u32)(maj->first) + sizeof(struct liballoc_minor));
			ALIGN(p);
			liballoc_unlock();
			return p;
		}
#endif

#ifdef USE_CASE4
		min = maj->first;

		while (min != NULL) {
			if (min->next == NULL) {
				diff = (u32)(maj) + maj->size;
				diff -= (u32)min;
				diff -= sizeof(struct liballoc_minor);
				diff -= min->size;
				if (diff >= (size + sizeof(struct liballoc_minor))) {
					min->next = (struct liballoc_minor
							     *)((u32)min +
								sizeof(struct liballoc_minor) +
								min->size);
					min->next->prev = min;
					min = min->next;
					min->next = NULL;
					min->magic = LIBALLOC_MAGIC;
					min->block = maj;
					min->size = size;
					min->req_size = req_size;
					maj->usage += size + sizeof(struct liballoc_minor);
					l_inuse += size;
					p = (void *)((u32)min + sizeof(struct liballoc_minor));
					ALIGN(p);
					liballoc_unlock();
					return p;
				}
			}

			if (min->next != NULL) {
				diff = (u32)(min->next);
				diff -= (u32)min;
				diff -= sizeof(struct liballoc_minor);
				diff -= min->size;

				if (diff >= (size + sizeof(struct liballoc_minor))) {
					new_min = (struct liballoc_minor
							   *)((u32)min +
							      sizeof(struct liballoc_minor) +
							      min->size);
					new_min->magic = LIBALLOC_MAGIC;
					new_min->next = min->next;
					new_min->prev = min;
					new_min->size = size;
					new_min->req_size = req_size;
					new_min->block = maj;
					min->next->prev = new_min;
					min->next = new_min;
					maj->usage += size + sizeof(struct liballoc_minor);
					l_inuse += size;
					p = (void *)((u32)new_min + sizeof(struct liballoc_minor));
					ALIGN(p);
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

	return NULL;
}

// Definitely improveable
void *valloc(u32 req_size)
{
	u32 mask = l_page_size - 1;
	u32 mem = malloc(req_size + l_page_size);
	return (void *)((mem + mask) & ~mask);
}

void free(void *ptr)
{
	struct liballoc_minor *min;
	struct liballoc_major *maj;

	if (ptr == NULL) {
		l_warning_count += 1;
		return;
	}

	UNALIGN(ptr);
	liballoc_lock();

	min = (struct liballoc_minor *)((u32)ptr - sizeof(struct liballoc_minor));

	if (min->magic != LIBALLOC_MAGIC) {
		l_error_count += 1;

		if (((min->magic & 0xFFFFFF) == (LIBALLOC_MAGIC & 0xFFFFFF)) ||
		    ((min->magic & 0xFFFF) == (LIBALLOC_MAGIC & 0xFFFF)) ||
		    ((min->magic & 0xFF) == (LIBALLOC_MAGIC & 0xFF))) {
			l_possible_overruns += 1;
		}

		liballoc_unlock();
		return;
	}

	maj = min->block;
	l_inuse -= min->size;
	maj->usage -= (min->size + sizeof(struct liballoc_minor));
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
		l_allocated -= maj->size;
		liballoc_free(maj, maj->pages);
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

void *calloc(u32 nobj, u32 size)
{
	int real_size;
	void *p;

	real_size = nobj * size;

	p = malloc(real_size);

	liballoc_memset(p, 0, real_size);

	return p;
}

void *realloc(void *p, u32 size)
{
	void *ptr;
	struct liballoc_minor *min;
	u32 real_size;

	if (size == 0) {
		free(p);
		return NULL;
	}

	if (p == NULL)
		return malloc(size);

	ptr = p;
	UNALIGN(ptr);
	liballoc_lock();
	min = (struct liballoc_minor *)((u32)ptr - sizeof(struct liballoc_minor));

	if (min->magic != LIBALLOC_MAGIC) {
		l_error_count += 1;
		if (((min->magic & 0xFFFFFF) == (LIBALLOC_MAGIC & 0xFFFFFF)) ||
		    ((min->magic & 0xFFFF) == (LIBALLOC_MAGIC & 0xFFFF)) ||
		    ((min->magic & 0xFF) == (LIBALLOC_MAGIC & 0xFF))) {
			l_possible_overruns += 1;
		}

		liballoc_unlock();
		return NULL;
	}

	real_size = min->req_size;

	if (real_size >= size) {
		min->req_size = size;
		liballoc_unlock();
		return p;
	}

	liballoc_unlock();

	ptr = malloc(size);
	liballoc_memcpy(ptr, p, real_size);
	free(p);

	return ptr;
}
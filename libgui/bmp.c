// MIT License, Copyright (c) 2020 Marvin Borner

#include <bmp.h>
#include <def.h>
#include <mem.h>
#include <print.h>
#include <sys.h>

struct bmp *bmp_load(const char *path)
{
	void *buf = read(path);
	if (!buf)
		return NULL;

	struct bmp_header *h = buf;
	if (h->signature[0] != 'B' || h->signature[1] != 'M')
		return NULL;

	struct bmp_info *info = (struct bmp_info *)((u32)buf + sizeof(*h));
	struct bmp *bmp = malloc(sizeof(*bmp));
	bmp->width = info->width;
	bmp->height = info->height;
	bmp->data = (u8 *)((u32)buf + h->offset);
	bmp->bpp = info->bpp;
	bmp->pitch = bmp->width * (bmp->bpp >> 3);

	return bmp;
}

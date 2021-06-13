// MIT License, Copyright (c) 2020 Marvin Borner
// Some GFX functions
// TODO: Better support for bpp < 32
// TODO: Use efficient redrawing

#include <assert.h>
#include <crypto.h>
#include <libgui/gfx.h>
#include <libgui/msg.h>
#include <libgui/png.h>
#include <libgui/psf.h>
#include <list.h>
#include <math.h>
#include <mem.h>
#include <str.h>
#include <sys.h>

// TODO: Move to some global config file
#define FONT_COUNT 6
#define FONT_8_PATH "/font/spleen-5x8.psfu"
#define FONT_12_PATH "/font/spleen-6x12.psfu"
#define FONT_16_PATH "/font/spleen-8x16.psfu"
#define FONT_24_PATH "/font/spleen-12x24.psfu"
#define FONT_32_PATH "/font/spleen-16x32.psfu"
#define FONT_64_PATH "/font/spleen-32x64.psfu"

struct gfx_font *fonts[FONT_COUNT] = { 0 };

static void load_font(enum font_type font_type)
{
	if (fonts[font_type])
		return;

	const char *path = NULL;

	switch (font_type) {
	case FONT_8:
		path = FONT_8_PATH;
		break;
	case FONT_12:
		path = FONT_12_PATH;
		break;
	case FONT_16:
		path = FONT_16_PATH;
		break;
	case FONT_24:
		path = FONT_24_PATH;
		break;
	case FONT_32:
		path = FONT_32_PATH;
		break;
	case FONT_64:
		path = FONT_64_PATH;
		break;
	default:
		return;
	}

	fonts[font_type] = psf_parse(sread(path));
	assert(fonts[font_type]);
}

static void free_fonts(void)
{
	for (u8 i = 0; i < FONT_COUNT; i++) {
		if (fonts[i]) {
			free(fonts[i]->raw);
			free(fonts[i]);
		}
	}
}

static void write_char(struct gfx_context *ctx, vec2 pos, struct gfx_font *font, u32 c, char ch)
{
	u8 bypp = ctx->bpp >> 3;

	char *draw = (char *)&ctx->fb[pos.x * bypp + pos.y * ctx->pitch];

	u32 stride = font->char_size / font->size.y;
	for (u32 cy = 0; cy < font->size.y; cy++) {
		for (u32 cx = 0; cx < font->size.x; cx++) {
			u8 bits = font->chars[ch * font->char_size + cy * stride + cx / 8];
			u8 bit = bits >> (7 - cx % 8) & 1;
			if (bit) {
				draw[bypp * cx] = GET_BLUE(c);
				draw[bypp * cx + 1] = GET_GREEN(c);
				draw[bypp * cx + 2] = GET_RED(c);
				draw[bypp * cx + 3] = GET_ALPHA(c);
			}
		}
		draw += ctx->pitch;
	}
}

struct gfx_context *gfx_new_ctx(struct gfx_context *ctx, vec2 size, u8 bpp)
{
	ctx->size = size;
	ctx->bpp = bpp;
	ctx->pitch = size.x * (bpp >> 3);
	ctx->bytes = ctx->pitch * ctx->size.y;
	ctx->fb = zalloc(ctx->bytes);
	return ctx;
}

struct gfx_context *gfx_clone(struct gfx_context *ctx)
{
	struct gfx_context *new = zalloc(sizeof(*new));
	gfx_new_ctx(new, ctx->size, ctx->bpp);
	assert(new->bytes == ctx->bytes);
	memcpy(new->fb, ctx->fb, ctx->bytes);
	return new;
}

/**
 * Font/text
 */

// On-demand font loading
static u8 fonts_loaded = 0;
struct gfx_font *gfx_resolve_font(enum font_type font_type)
{
	if (!fonts_loaded) {
		fonts_loaded = 1;
		atexit(free_fonts);
	}

	if (!fonts[font_type])
		load_font(font_type);
	return fonts[font_type];
}

void gfx_write_char(struct gfx_context *ctx, vec2 pos, enum font_type font_type, u32 c, char ch)
{
	struct gfx_font *font = gfx_resolve_font(font_type);
	write_char(ctx, pos, font, c, ch);
}

void gfx_write(struct gfx_context *ctx, vec2 pos, enum font_type font_type, u32 c, const char *text)
{
	struct gfx_font *font = gfx_resolve_font(font_type);
	u32 cnt = 0;
	for (u32 i = 0; i < strlen(text); i++) {
		// TODO: Should this be here?
		if (text[i] == '\r') {
			cnt = 0;
		} else if (text[i] == '\n') {
			cnt = 0;
			pos.y += font->size.y;
		} else if (text[i] == '\t') {
			cnt += 4;
		} else {
			// TODO: Overflow on single line input
			if ((cnt + 1) * font->size.x > ctx->size.x) {
				cnt = 0;
				pos.y += font->size.y;
			}
			write_char(ctx, vec2(pos.x + cnt * font->size.x, pos.y), font, c, text[i]);
			cnt++;
		}
	}
}

/**
 * Image drawing/caching
 */

struct gfx_image_cache {
	u32 hash;
	struct gfx_context *ctx;
};

struct list *gfx_image_cache_list = NULL;

static void gfx_image_cache_clear(void)
{
	struct node *iterator = gfx_image_cache_list->head;
	while (iterator) {
		struct gfx_image_cache *cache = iterator->data;
		free(cache->ctx->fb);
		free(cache->ctx);
		free(cache);
		iterator = iterator->next;
	}

	list_destroy(gfx_image_cache_list);
}

static void gfx_image_cache_init(void)
{
	if (!gfx_image_cache_list) {
		gfx_image_cache_list = list_new();
		atexit(gfx_image_cache_clear);
	}
}

static struct gfx_context *gfx_image_cache_get(const char *path)
{
	gfx_image_cache_init();

	u32 hash = crc32(0, path, strlen(path));

	struct node *iterator = gfx_image_cache_list->head;
	while (iterator) {
		struct gfx_image_cache *cache = iterator->data;
		if (cache->hash == hash)
			return cache->ctx;
		iterator = iterator->next;
	}

	return NULL;
}

static void gfx_image_cache_save(const char *path, struct gfx_context *ctx)
{
	gfx_image_cache_init();

	struct gfx_image_cache *cache = zalloc(sizeof(*cache));
	cache->hash = crc32(0, path, strlen(path));
	cache->ctx = ctx;
	list_add(gfx_image_cache_list, cache);
}

void gfx_draw_image_filter(struct gfx_context *ctx, vec2 pos, vec2 size, enum gfx_filter filter,
			   const char *path)
{
	// TODO: Detect image type

	struct gfx_context *bmp = gfx_image_cache_get(path);

	if (!bmp) {
		bmp = zalloc(sizeof(*bmp));
		u32 error = png_decode32_file(&bmp->fb, &bmp->size.x, &bmp->size.y, path);
		if (error)
			err(1, "error %d: %s\n", error, png_error_text(error));

		bmp->bpp = 32;
		bmp->pitch = bmp->size.x * (bmp->bpp >> 3);
		bmp->bytes = bmp->size.y * bmp->pitch;
		gfx_image_cache_save(path, bmp);
	}

	// Scaling clones!
	bmp = gfx_scale(bmp, size);

	assert(bmp->size.x + pos.x <= ctx->size.x);
	assert(bmp->size.y + pos.y <= ctx->size.y);

	u8 bypp = bmp->bpp >> 3;
	u8 *srcfb = bmp->fb;
	u8 *destfb = &ctx->fb[pos.x * bypp + pos.y * ctx->pitch];
	for (u32 cy = 0; cy < bmp->size.y && cy + pos.y < ctx->size.y; cy++) {
		int diff = 0;
		for (u32 cx = 0; cx < bmp->size.x && cx + pos.x < ctx->size.x; cx++) {
			if (srcfb[bypp - 1]) {
				if (filter == GFX_FILTER_NONE) {
					destfb[0] = srcfb[2];
					destfb[1] = srcfb[1];
					destfb[2] = srcfb[0];
					destfb[3] = srcfb[3];
				} else if (filter == GFX_FILTER_INVERT) {
					destfb[0] = 0xff - srcfb[2];
					destfb[1] = 0xff - srcfb[1];
					destfb[2] = 0xff - srcfb[0];
					destfb[3] = srcfb[3];
				}
			}

			srcfb += bypp;
			destfb += bypp;
			diff += bypp;
		}
		srcfb += bmp->pitch - diff;
		destfb += ctx->pitch - diff;
	}

	free(bmp);
	free(bmp->fb);
}

void gfx_draw_image(struct gfx_context *ctx, vec2 pos, vec2 size, const char *path)
{
	gfx_draw_image_filter(ctx, pos, size, GFX_FILTER_NONE, path);
}

void gfx_load_wallpaper(struct gfx_context *ctx, const char *path)
{
	gfx_draw_image(ctx, vec2(0, 0), ctx->size, path);
}

/**
 * Context transformations
 */

// Using bilinear interpolation
struct gfx_context *gfx_scale(struct gfx_context *ctx, vec2 size)
{
	if (vec2_eq(ctx->size, size))
		return gfx_clone(ctx);

	u8 bypp = ctx->bpp >> 3;

	struct gfx_context *new = zalloc(sizeof(*new));
	gfx_new_ctx(new, size, ctx->bpp);

	for (u32 x = 0, y = 0; y < size.y; x++) {
		if (x > size.x) {
			x = 0;
			y++;
		}

		f32 gx = x / (f32)size.x * (ctx->size.x - 1);
		f32 gy = y / (f32)size.y * (ctx->size.y - 1);
		u32 gxi = (u32)gx;
		u32 gyi = (u32)gy;

		u32 a = *(u32 *)&ctx->fb[(gxi + 0) * bypp + (gyi + 0) * ctx->pitch];
		u32 b = *(u32 *)&ctx->fb[(gxi + 1) * bypp + (gyi + 0) * ctx->pitch];
		u32 c = *(u32 *)&ctx->fb[(gxi + 0) * bypp + (gyi + 1) * ctx->pitch];
		u32 d = *(u32 *)&ctx->fb[(gxi + 1) * bypp + (gyi + 1) * ctx->pitch];

		u32 color = 0;
		for (u8 i = 0; i < bypp; i++) {
			color |= ((u32)blerpf(GET_COLOR(a, i), GET_COLOR(b, i), GET_COLOR(c, i),
					      GET_COLOR(d, i), gx - gxi, gy - gyi))
				 << (i << 3);
		}

		gfx_draw_pixel(new, vec2(x, y), color);
	}

	return new;
}

/**
 * General drawing functions
 */

void gfx_draw_pixel(struct gfx_context *ctx, vec2 pos, u32 c)
{
	u8 bypp = ctx->bpp >> 3;
	u8 *draw = &ctx->fb[pos.x * bypp + pos.y * ctx->pitch];
	draw[0] = GET_BLUE(c);
	draw[1] = GET_GREEN(c);
	draw[2] = GET_RED(c);
	draw[3] = GET_ALPHA(c);
}

void gfx_draw_rectangle(struct gfx_context *ctx, vec2 pos1, vec2 pos2, u32 c)
{
	assert(pos1.x <= pos2.x && pos1.y <= pos2.y);
	u8 bypp = ctx->bpp >> 3;
	u8 *draw = &ctx->fb[pos1.x * bypp + pos1.y * ctx->pitch];
	for (u32 i = 0; i < pos2.y - pos1.y; i++) {
		for (u32 j = 0; j < pos2.x - pos1.x; j++) {
			draw[bypp * j] = GET_BLUE(c);
			draw[bypp * j + 1] = GET_GREEN(c);
			draw[bypp * j + 2] = GET_RED(c);
			draw[bypp * j + 3] = GET_ALPHA(c);
		}
		draw += ctx->pitch;
	}
}

void gfx_draw_border(struct gfx_context *ctx, u32 width, u32 c)
{
	if (width <= 0)
		return;

	u8 bypp = ctx->bpp >> 3;
	u8 *draw = ctx->fb;
	for (u32 i = 0; i < ctx->size.y; i++) {
		for (u32 j = 0; j < ctx->size.x; j++) {
			if (j <= width - 1 || i <= width - 1 || j - ctx->size.x + width <= width ||
			    i - ctx->size.y + width <= width) {
				draw[bypp * j + 0] = GET_BLUE(c);
				draw[bypp * j + 1] = GET_GREEN(c);
				draw[bypp * j + 2] = GET_RED(c);
				draw[bypp * j + 3] = GET_ALPHA(c);
			}
		}
		draw += ctx->pitch;
	}
}

// Using Bresenham's algorithm
// TODO: Better line scaling
void gfx_draw_line(struct gfx_context *ctx, vec2 pos1, vec2 pos2, u32 scale, u32 c)
{
	int dx = ABS(pos2.x - pos1.x), sx = pos1.x < pos2.x ? 1 : -1;
	int dy = ABS(pos2.y - pos1.y), sy = pos1.y < pos2.y ? 1 : -1;
	int err = (dx > dy ? dx : -dy) / 2, e2;

	while (1) {
		gfx_draw_rectangle(ctx, pos1, vec2_add(pos1, vec2(scale, scale)), c);
		if (pos1.x == pos2.x && pos1.y == pos2.y)
			break;
		e2 = err;
		if (e2 > -dx) {
			err -= dy;
			pos1.x += sx;
		}
		if (e2 < dy) {
			err += dx;
			pos1.y += sy;
		}
	}
}

void gfx_copy(struct gfx_context *dest, struct gfx_context *src, vec2 pos, vec2 size)
{
	u8 bypp = dest->bpp >> 3;
	u8 *srcfb = &src->fb[pos.x * bypp + pos.y * src->pitch];
	u8 *destfb = &dest->fb[pos.x * bypp + pos.y * dest->pitch];
	for (u32 cy = 0; cy < size.y; cy++) {
		memcpy(destfb, srcfb, size.x * bypp);
		srcfb += src->pitch;
		destfb += dest->pitch;
	}
}

// TODO: Support alpha values other than 0x0 and 0xff (blending)
// TODO: Optimize!
HOT void gfx_ctx_on_ctx(struct gfx_context *dest, struct gfx_context *src, vec2 pos, u8 alpha)
{
	// TODO: Some kind of alpha-acknowledging memcpy?
	if (!alpha && src->size.x == dest->size.x && src->size.y == dest->size.y) {
		memcpy(dest->fb, src->fb, dest->pitch * dest->size.y);
		return;
	}

	if (src->size.x > dest->size.x || src->size.y > dest->size.y)
		return;

	// TODO: Negative x and y
	u8 bypp = dest->bpp >> 3;
	u8 *srcfb = src->fb;
	u8 *destfb = &dest->fb[pos.x * bypp + pos.y * dest->pitch];
	for (u32 cy = 0; cy < src->size.y && cy + pos.y < dest->size.y; cy++) {
		int diff = 0;
		for (u32 cx = 0; cx < src->size.x && cx + pos.x < dest->size.x; cx++) {
			if (!alpha || srcfb[bypp - 1])
				memcpy(destfb, srcfb, bypp);

			srcfb += bypp;
			destfb += bypp;
			diff += bypp;
		}
		srcfb += src->pitch - diff;
		destfb += dest->pitch - diff;
	}
}

void gfx_clear(struct gfx_context *ctx)
{
	memset(ctx->fb, 0, ctx->bytes);
}

void gfx_fill(struct gfx_context *ctx, u32 c)
{
	gfx_draw_rectangle(ctx, vec2(0, 0), vec2(ctx->size.x, ctx->size.y), c);
}

int gfx_font_height(enum font_type font_type)
{
	struct gfx_font *font = gfx_resolve_font(font_type);
	return font->size.y;
}

int gfx_font_width(enum font_type font_type)
{
	struct gfx_font *font = gfx_resolve_font(font_type);
	return font->size.x;
}

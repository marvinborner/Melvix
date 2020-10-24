// MIT License, Copyright (c) 2020 Marvin Borner
// Some GFX functions
// TODO: Better support for bpp < 32

#include <assert.h>
#include <bmp.h>
#include <gfx.h>
#include <mem.h>
#include <psf.h>
#include <str.h>
#include <sys.h>
#include <vesa.h>

struct font *font;

static void write_char(struct context *ctx, int x, int y, u32 c, char ch)
{
	int bypp = ctx->bpp >> 3;

	int pos = x * bypp + y * ctx->pitch;
	char *draw = (char *)&ctx->fb[pos];

	u32 stride = font->char_size / font->height;
	for (int cy = 0; cy < font->height; cy++) {
		for (int cx = 0; cx < font->width; cx++) {
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

static void draw_rectangle(struct context *ctx, int x1, int y1, int x2, int y2, u32 c)
{
	int bypp = ctx->bpp >> 3;
	u8 *draw = &ctx->fb[x1 * bypp + y1 * ctx->pitch];
	for (int i = 0; i < y2 - y1; i++) {
		for (int j = 0; j < x2 - x1; j++) {
			draw[bypp * j] = GET_BLUE(c);
			draw[bypp * j + 1] = GET_GREEN(c);
			draw[bypp * j + 2] = GET_RED(c);
			draw[bypp * j + 3] = GET_ALPHA(c);
		}
		draw += ctx->pitch;
	}
}

void gfx_write_char(struct context *ctx, int x, int y, u32 c, char ch)
{
	write_char(ctx, x, y, c, ch);
	gfx_redraw();
}

void gfx_write(struct context *ctx, int x, int y, u32 c, char *text)
{
	for (u32 i = 0; i < strlen(text); i++) {
		write_char(ctx, x + i * font->width, y, c, text[i]);
	}
	gfx_redraw();
}

void gfx_load_image(struct context *ctx, char *path, int x, int y)
{
	// TODO: Support x, y
	struct bmp *bmp = bmp_load(path);
	assert(bmp && bmp->width + x <= ctx->width);
	assert(bmp && bmp->height + y <= ctx->height);

	// TODO: Support padding with odd widths
	int bypp = bmp->bpp >> 3;
	u8 *srcfb = &bmp->data[bypp + (bmp->height - 1) * bmp->pitch];
	u8 *destfb = &ctx->fb[bypp];
	for (u32 cy = 0; cy < bmp->height; cy++) {
		memcpy(destfb, srcfb, bmp->pitch);
		srcfb -= bmp->pitch;
		destfb += ctx->pitch;
	}
	gfx_redraw();
}

void gfx_load_wallpaper(struct context *ctx, char *path)
{
	gfx_load_image(ctx, path, 0, 0);
}

void gfx_copy(struct context *dest, struct context *src, int x, int y, u32 width, u32 height)
{
	int bypp = dest->bpp >> 3;
	u8 *srcfb = &src->fb[x * bypp + y * src->pitch];
	u8 *destfb = &dest->fb[x * bypp + y * dest->pitch];
	for (u32 cy = 0; cy < height; cy++) {
		memcpy(destfb, srcfb, width * (dest->bpp >> 3));
		srcfb += src->pitch;
		destfb += dest->pitch;
	}
}

// TODO: Support alpha values other than 0x0 and 0xff (blending)
// TODO: Optimize!
void gfx_ctx_on_ctx(struct context *dest, struct context *src, int x, int y)
{
	if (src->width == dest->width && src->height == dest->height && src->x == 0 &&
	    dest->y == 0) {
		memcpy(dest->fb, src->fb, dest->pitch * dest->height);
		return;
	}

	if (src->width > dest->width || src->height > dest->height)
		return;

	// TODO: Negative x and y
	int bypp = dest->bpp >> 3;
	u8 *srcfb = src->fb;
	u8 *destfb = &dest->fb[x * bypp + y * dest->pitch];
	for (u32 cy = 0; cy < src->height && cy + y < dest->height; cy++) {
		for (u32 cx = 0; cx < src->width && cx + x < dest->width; cx++) {
			if (srcfb[bypp * cx + 3]) {
				destfb[bypp * cx + 0] = srcfb[bypp * cx + 0];
				destfb[bypp * cx + 1] = srcfb[bypp * cx + 1];
				destfb[bypp * cx + 2] = srcfb[bypp * cx + 2];
				destfb[bypp * cx + 3] = srcfb[bypp * cx + 3];
			}
		}
		srcfb += src->pitch;
		destfb += dest->pitch;
	}
}

void gfx_draw_rectangle(struct context *ctx, int x1, int y1, int x2, int y2, u32 c)
{
	draw_rectangle(ctx, x1, y1, x2, y2, c);
	gfx_redraw();
}

void gfx_fill(struct context *ctx, u32 c)
{
	draw_rectangle(ctx, 0, 0, ctx->width, ctx->height, c);
	gfx_redraw();
}

void gfx_border(struct context *ctx, u32 c, u32 width)
{
	if (width <= 0)
		return;

	int bypp = ctx->bpp >> 3;
	u8 *draw = ctx->fb;
	for (u32 i = 0; i < ctx->height; i++) {
		for (u32 j = 0; j < ctx->width; j++) {
			if (j <= width - 1 || i <= width - 1 ||
			    j - ctx->width + width + 1 <= width ||
			    i - ctx->height + width <= width) {
				draw[bypp * j + 0] = GET_BLUE(c);
				draw[bypp * j + 1] = GET_GREEN(c);
				draw[bypp * j + 2] = GET_RED(c);
				draw[bypp * j + 3] = GET_ALPHA(c);
			}
		}
		draw += ctx->pitch;
	}
	gfx_redraw();
}

int gfx_font_height()
{
	return font->height;
}

int gfx_font_width()
{
	return font->width;
}

void gfx_init(char *font_path)
{
	font = psf_parse(read(font_path));
	assert(font);
}

// MIT License, Copyright (c) 2020 Marvin Borner
// Some GFX functions
// TODO: Better support for bpp < 32
// TODO: Use efficient redrawing

#include <assert.h>
#include <bmp.h>
#include <gfx.h>
#include <mem.h>
#include <msg.h>
#include <png.h>
#include <psf.h>
#include <str.h>
#include <sys.h>
#include <vesa.h>

// TODO: Move to some global config file
#define FONT_COUNT 6
#define FONT_8_PATH "/font/spleen-5x8.psfu"
#define FONT_12_PATH "/font/spleen-6x12.psfu"
#define FONT_16_PATH "/font/spleen-8x16.psfu"
#define FONT_24_PATH "/font/spleen-12x24.psfu"
#define FONT_32_PATH "/font/spleen-16x32.psfu"
#define FONT_64_PATH "/font/spleen-32x64.psfu"

struct font *fonts[FONT_COUNT] = { 0 };

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

static void write_char(struct context *ctx, vec2 pos, struct font *font, u32 c, char ch)
{
	int bypp = ctx->bpp >> 3;

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

static void draw_rectangle(struct context *ctx, vec2 pos1, vec2 pos2, u32 c)
{
	int bypp = ctx->bpp >> 3;
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

struct context *gfx_new_ctx(struct context *ctx)
{
	/* struct message msg = { 0 }; */
	assert(0);
	/* assert(msg_send(pidof(WM_PATH), GFX_NEW_CONTEXT, ctx) > 0); */
	/* assert(msg_receive(&msg) > 0); */
	/* memcpy(ctx, msg.data, sizeof(*ctx)); */
	return ctx;
}

// On-demand font loading
struct font *gfx_resolve_font(enum font_type font_type)
{
	if (!fonts[font_type])
		load_font(font_type);
	return fonts[font_type];
}

void gfx_write_char(struct context *ctx, vec2 pos, enum font_type font_type, u32 c, char ch)
{
	struct font *font = gfx_resolve_font(font_type);
	write_char(ctx, pos, font, c, ch);
	/* gfx_redraw(); */
}

void gfx_write(struct context *ctx, vec2 pos, enum font_type font_type, u32 c, const char *text)
{
	struct font *font = gfx_resolve_font(font_type);
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
	/* gfx_redraw(); */
}

void gfx_load_image(struct context *ctx, vec2 pos, const char *path)
{
	// TODO: Support x, y
	// TODO: Detect image type
	struct bmp bmp = { 0 };

	u32 error = png_decode32_file(&bmp.data, &bmp.size.x, &bmp.size.y, path);
	if (error)
		err(1, "error %u: %s\n", error, png_error_text(error));

	assert(bmp.size.x + pos.x <= ctx->size.x);
	assert(bmp.size.y + pos.y <= ctx->size.y);

	bmp.bpp = 32;
	bmp.pitch = bmp.size.x * (bmp.bpp >> 3);

	// TODO: Fix reversed png in decoder
	int bypp = bmp.bpp >> 3;
	// u8 *srcfb = &bmp->data[bypp + (bmp->size.y - 1) * bmp->pitch];
	u8 *srcfb = bmp.data;
	u8 *destfb = &ctx->fb[bypp];
	for (u32 cy = 0; cy < bmp.size.y; cy++) {
		memcpy(destfb, srcfb, bmp.pitch);
		// srcfb -= bmp->pitch;
		srcfb += bmp.pitch;
		destfb += ctx->pitch;
	}
}

void gfx_load_wallpaper(struct context *ctx, const char *path)
{
	gfx_load_image(ctx, vec2(0, 0), path);
}

void gfx_copy(struct context *dest, struct context *src, vec2 pos, vec2 size)
{
	int bypp = dest->bpp >> 3;
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
void gfx_ctx_on_ctx(struct context *dest, struct context *src, vec2 pos)
{
	if (src->size.x == dest->size.x && src->size.y == dest->size.y) {
		memcpy(dest->fb, src->fb, dest->pitch * dest->size.y);
		return;
	}

	if (src->size.x > dest->size.x || src->size.y > dest->size.y)
		return;

	// TODO: Negative x and y
	int bypp = dest->bpp >> 3;
	u8 *srcfb = src->fb;
	u8 *destfb = &dest->fb[pos.x * bypp + pos.y * dest->pitch];
	for (u32 cy = 0; cy < src->size.y && cy + pos.y < dest->size.y; cy++) {
		int diff = 0;
		for (u32 cx = 0; cx < src->size.x && cx + pos.x < dest->size.x; cx++) {
			if (srcfb[bypp - 1])
				memcpy(destfb, srcfb, bypp);

			srcfb += bypp;
			destfb += bypp;
			diff += bypp;
		}
		srcfb += src->pitch - diff;
		destfb += dest->pitch - diff;
	}
}

void gfx_draw_rectangle(struct context *ctx, vec2 pos1, vec2 pos2, u32 c)
{
	draw_rectangle(ctx, pos1, pos2, c);
	/* gfx_redraw(); */
}

void gfx_fill(struct context *ctx, u32 c)
{
	draw_rectangle(ctx, vec2(0, 0), vec2(ctx->size.x, ctx->size.y), c);
	/* gfx_redraw(); */
}

void gfx_border(struct context *ctx, u32 c, u32 width)
{
	if (width <= 0)
		return;

	int bypp = ctx->bpp >> 3;
	u8 *draw = ctx->fb;
	for (u32 i = 0; i < ctx->size.y; i++) {
		for (u32 j = 0; j < ctx->size.x; j++) {
			if (j <= width - 1 || i <= width - 1 ||
			    j - ctx->size.x + width + 1 <= width ||
			    i - ctx->size.y + width <= width) {
				draw[bypp * j + 0] = GET_BLUE(c);
				draw[bypp * j + 1] = GET_GREEN(c);
				draw[bypp * j + 2] = GET_RED(c);
				draw[bypp * j + 3] = GET_ALPHA(c);
			}
		}
		draw += ctx->pitch;
	}
	/* gfx_redraw(); */
}

int gfx_font_height(enum font_type font_type)
{
	struct font *font = gfx_resolve_font(font_type);
	return font->size.y;
}

int gfx_font_width(enum font_type font_type)
{
	struct font *font = gfx_resolve_font(font_type);
	return font->size.x;
}

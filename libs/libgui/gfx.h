// MIT License, Copyright (c) 2020 Marvin Borner
// Some GFX functions

#ifndef GFX_H
#define GFX_H

#include <def.h>
#include <sys.h>
#include <vec.h>

#define WM_PATH "wm"

#define GET_COLOR(color, n) (((color) >> ((n) << 3)) & 0xff)
#define GET_ALPHA(color) (GET_COLOR((color), 3))
#define GET_RED(color) (GET_COLOR((color), 2))
#define GET_GREEN(color) (GET_COLOR((color), 1))
#define GET_BLUE(color) (GET_COLOR((color), 0))

#define COLOR_TRANSPARENT 0x00000000
#define COLOR_INVISIBLE 0x00000000
#define COLOR_BLACK 0xff0f0f0f
#define COLOR_RED 0xfff07f7f
#define COLOR_GREEN 0xff7ff088
#define COLOR_YELLOW 0xffeef07f
#define COLOR_BLUE 0xff7facf0
#define COLOR_MAGENTA 0xffd67ff0
#define COLOR_CYAN 0xff7fe7f0
#define COLOR_WHITE 0xffe9e9e9
#define COLOR_BRIGHT_BLACK 0xff928374
#define COLOR_BRIGHT_RED 0xffed9a9a
#define COLOR_BRIGHT_GREEN 0xff9ef0a5
#define COLOR_BRIGHT_YELLOW 0xffe7e897
#define COLOR_BRIGHT_BLUE 0xff98b9eb
#define COLOR_BRIGHT_MAGENTA 0xffd196e3
#define COLOR_BRIGHT_CYAN 0xff94dae0
#define COLOR_BRIGHT_WHITE 0xffe3e3e3
#define COLOR_FG COLOR_WHITE
#define COLOR_BG COLOR_BLACK

#define WF_DEFAULT (0 << 0)
#define WF_NO_FOCUS (1 << 0)
#define WF_NO_DRAG (1 << 1)
#define WF_NO_RESIZE (1 << 2)
#define WF_NO_FB (1 << 3)
#define WF_NO_WINDOW (1 << 4)
#define WF_ALPHA (1 << 5)
#define WF_BAR (1 << 6)

#define GFX_NON_ALPHA 0
#define GFX_ALPHA 1

/**
 * Useful macros
 */

#define GFX_CENTER_IN(a, b) (ABS((a) - (b)) / 2)
#define GFX_RECT(pos, size) ((struct gfx_rect){ .pos = (pos), .size = (size) })
#define GFX_IN_RECT(rect, p)                                                                       \
	((p).x >= (rect).pos.x && (p).x < (rect).pos.x + (rect).size.x && (p).y >= (rect).pos.y && \
	 (p).y < (rect).pos.y + (rect).size.y)

/**
 * Structures
 */

enum font_type { FONT_8, FONT_12, FONT_16, FONT_24, FONT_32, FONT_64 };
enum gfx_filter {
	GFX_FILTER_NONE,
	GFX_FILTER_INVERT,
};

// Generalized font struct
struct gfx_font {
	void *raw;
	char *chars;
	vec2 size;
	int char_size;
};

struct gfx_context {
	vec2 size;
	u8 *fb;
	u32 bpp;
	u32 pitch;
	u32 bytes;
};

struct gfx_rect {
	vec2 pos; // Upper left
	vec2 size;
};

struct gfx_context *gfx_new_ctx(struct gfx_context *ctx, vec2 size, u8 bpp) NONNULL;
struct gfx_context *gfx_clone(struct gfx_context *ctx) NONNULL;

/**
 * Text stuff
 */

struct gfx_font *gfx_resolve_font(enum font_type font_type);
void gfx_write_char(struct gfx_context *ctx, vec2 pos, enum font_type font_type, u32 c,
		    char ch) NONNULL;
void gfx_write(struct gfx_context *ctx, vec2 pos, enum font_type font_type, u32 c,
	       const char *text) NONNULL;

int gfx_font_height(enum font_type);
int gfx_font_width(enum font_type);

/**
 * Image loading
 */

void gfx_draw_image(struct gfx_context *ctx, vec2 pos, vec2 size, const char *path) NONNULL;
void gfx_draw_image_filter(struct gfx_context *ctx, vec2 pos, vec2 size, enum gfx_filter filter,
			   const char *path) NONNULL;
void gfx_load_wallpaper(struct gfx_context *ctx, const char *path) NONNULL;

/**
 * Context copying
 */

void gfx_copy(struct gfx_context *dest, struct gfx_context *src, vec2 pos, vec2 size) NONNULL;
void gfx_ctx_on_ctx(struct gfx_context *dest, struct gfx_context *src, vec2 pos, u8 alpha) NONNULL;

/**
 * Context transformations
 */

struct gfx_context *gfx_scale(struct gfx_context *ctx, vec2 size) NONNULL;

/**
 * Drawing functions
 */

void gfx_draw_pixel(struct gfx_context *ctx, vec2 pos1, u32 c);
void gfx_draw_rectangle(struct gfx_context *ctx, vec2 pos1, vec2 pos2, u32 c) NONNULL;
void gfx_draw_line(struct gfx_context *ctx, vec2 pos1, vec2 pos2, u32 scale, u32 c);

void gfx_clear(struct gfx_context *ctx);
void gfx_fill(struct gfx_context *ctx, u32 c) NONNULL;
void gfx_draw_border(struct gfx_context *ctx, u32 width, u32 c) NONNULL;

#endif

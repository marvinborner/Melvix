// MIT License, Copyright (c) 2020 Marvin Borner
// Some GFX functions

#ifndef GFX_H
#define GFX_H

#include <def.h>
#include <libgui/vesa.h>
#include <sys.h>
#include <vec.h>

#define WM_PATH "/bin/wm"

#define GET_ALPHA(color) ((color >> 24) & 0x000000FF)
#define GET_RED(color) ((color >> 16) & 0x000000FF)
#define GET_GREEN(color) ((color >> 8) & 0x000000FF)
#define GET_BLUE(color) ((color >> 0) & 0X000000FF)

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
/* #define WF_RELATIVE (1 << 4) */

enum font_type { FONT_8, FONT_12, FONT_16, FONT_24, FONT_32, FONT_64 };

// Generalized font struct
struct font {
	char *chars;
	vec2 size;
	int char_size;
};

struct context {
	vec2 size;
	u8 *fb;
	u32 bpp;
	u32 pitch;
	u32 bytes;
};

struct context *gfx_new_ctx(struct context *ctx);
struct font *gfx_resolve_font(enum font_type font_type);
void gfx_write_char(struct context *ctx, vec2 pos, enum font_type font_type, u32 c, char ch);
void gfx_write(struct context *ctx, vec2 pos, enum font_type font_type, u32 c, const char *text);
void gfx_load_image(struct context *ctx, vec2 pos, const char *path);
void gfx_load_wallpaper(struct context *ctx, const char *path);
void gfx_copy(struct context *dest, struct context *src, vec2 pos, vec2 size);
void gfx_ctx_on_ctx(struct context *dest, struct context *src, vec2 pos);
void gfx_draw_rectangle(struct context *ctx, vec2 pos1, vec2 pos2, u32 c);
void gfx_fill(struct context *ctx, u32 c);
void gfx_border(struct context *ctx, u32 c, u32 width);

int gfx_font_height(enum font_type);
int gfx_font_width(enum font_type);

/**
 * Wrappers
 */

/* #define gfx_redraw()                                                                               \ */
/* 	(msg_send(pidof(WM_PATH), GFX_REDRAW, NULL)) // TODO: Partial redraw (optimization) */
/* #define gfx_redraw_focused() (msg_send(pidof(WM_PATH), GFX_REDRAW_FOCUSED, NULL)) */

#endif

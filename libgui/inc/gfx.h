// MIT License, Copyright (c) 2020 Marvin Borner
// Some GFX functions

#ifndef GFX_H
#define GFX_H

#include <def.h>
#include <gui.h>
#include <sys.h>
#include <vesa.h>

#define GET_ALPHA(color) ((color >> 24) & 0x000000FF)
#define GET_RED(color) ((color >> 16) & 0x000000FF)
#define GET_GREEN(color) ((color >> 8) & 0x000000FF)
#define GET_BLUE(color) ((color >> 0) & 0X000000FF)

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

enum message_type { WM_NEW_CONTEXT = EVENT_MAX + 1, WM_REDRAW, WM_KEYBOARD };

// Generalized font struct
struct font {
	char *chars;
	int height;
	int width;
	int char_size;
};

struct context {
	u32 pid;
	int x;
	int y;
	u32 width;
	u32 height;
	u8 *fb;
	u32 bpp;
	u32 pitch;
	int flags;
};

struct msg_keyboard {
	char ch;
	int press;
	int scancode;
};

void gfx_write_char(struct context *ctx, int x, int y, u32 c, char ch);
void gfx_write(struct context *ctx, int x, int y, u32 c, char *text);
void gfx_load_image(struct context *ctx, char *path, int x, int y);
void gfx_load_wallpaper(struct context *ctx, char *path);
void gfx_copy(struct context *dest, struct context *src, int x, int y, u32 width, u32 height);
void gfx_ctx_on_ctx(struct context *dest, struct context *src, int x, int y);
void gfx_draw_rectangle(struct context *ctx, int x1, int y1, int x2, int y2, u32 c);
void gfx_fill(struct context *ctx, u32 c);
void gfx_border(struct context *ctx, u32 c, u32 width);
void gfx_init(char *font_path);

int gfx_font_height();
int gfx_font_width();

/**
 * Wrappers
 */

#define gfx_new_ctx(ctx)                                                                           \
	(msg_send(2, WM_NEW_CONTEXT, (ctx)), (struct context *)msg_receive_loop()->data)
#define gfx_redraw() (msg_send(2, WM_REDRAW, NULL)) // TODO: Partial redraw (optimization)

#endif

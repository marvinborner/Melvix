// MIT License, Copyright (c) 2020 Marvin Borner
// Some GUI functions

#ifndef GUI_H
#define GUI_H

#include <def.h>
#include <sys.h>
#include <vesa.h>

#define GET_ALPHA(color) ((color >> 24) & 0x000000FF)
#define GET_RED(color) ((color >> 16) & 0x000000FF)
#define GET_GREEN(color) ((color >> 8) & 0x000000FF)
#define GET_BLUE(color) ((color >> 0) & 0X000000FF)
#define FG_COLOR 0xffabb2bf
#define BG_COLOR 0xff282c34

// Generalized font struct
struct font {
	char *chars;
	int height;
	int width;
	int char_size;
};

struct window {
	int x;
	int y;
	u32 width;
	u32 height;
	u8 *fb;
	u32 bpp;
	u32 pitch;
};

void gui_write_char(struct window *win, int x, int y, u32 c, char ch);
void gui_write(struct window *win, int x, int y, u32 c, char *text);
void gui_load_image(struct window *win, char *path, int x, int y);
void gui_load_wallpaper(struct window *win, char *path);
void gui_copy(struct window *dest, struct window *src, int x, int y, u32 width, u32 height);
void gui_win_on_win(struct window *dest, struct window *src, int x, int y);
void gui_draw_rectangle(struct window *win, int x1, int y1, int x2, int y2, u32 c);
void gui_fill(struct window *win, u32 c);
void gui_border(struct window *win, u32 c, u32 width);
void gui_init(char *font_path);

/**
 * Wrappers
 */

#define gui_new_window()                                                                           \
	(msg_send(2, MSG_NEW_WINDOW, NULL), (struct window *)msg_receive_loop()->data)
#define gui_redraw() (msg_send(2, MSG_REDRAW, NULL)) // TODO: Partial redraw (optimization)
#endif

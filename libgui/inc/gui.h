// MIT License, Copyright (c) 2020 Marvin Borner
// Some GUI functions

#ifndef GUI_H
#define GUI_H

#include <def.h>
#include <sys.h>
#include <vesa.h>

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

void gui_write_char(struct window *win, int x, int y, const u32 c[3], char ch);
void gui_write(struct window *win, int x, int y, const u32 c[3], char *text);
void gui_load_wallpaper(struct window *win, char *path);
void gui_win_on_win(struct window *dest, struct window *src, int x, int y);
void gui_draw_rectangle(struct window *win, int x1, int y1, int x2, int y2, const u32 c[3]);
void gui_fill(struct window *win, const u32 c[3]);
void gui_border(struct window *win, const u32 c[3], u32 width);
void gui_init(char *font_path);

/**
 * Wrappers
 */

#define gui_new_window()                                                                           \
	(msg_send(2, MSG_NEW_WINDOW, NULL), (struct window *)msg_receive_loop()->data)
#define gui_redraw() (msg_send(2, MSG_REDRAW, NULL)) // TODO: Partial redraw (optimisation)
#endif

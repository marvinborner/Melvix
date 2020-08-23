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
	u16 width;
	u16 height;
	struct vbe *vbe;
	u8 *fb;
};

void gui_draw_rectangle(struct window *win, int x1, int y1, int x2, int y2, const u32 color[3]);
void gui_fill(struct window *win, const u32 color[3]);
void gui_load_wallpaper(struct vbe *vbe, char *path);
void gui_init(char *font_path);

/**
 * Wrappers
 */

#define gui_new_window()                                                                           \
	(msg_send(1, MSG_NEW_WINDOW, NULL), (struct window *)msg_receive_loop()->data)

#endif

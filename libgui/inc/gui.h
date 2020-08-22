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

void gui_write(struct vbe *vbe, int x, int y, const u32 c[3], char *text);
void gui_init(char *font_path);

/**
 * Wrappers
 */
#define gui_new_window() msg_send(1, MSG_NEW_WINDOW, NULL);

#endif

// MIT License, Copyright (c) 2020 Marvin Borner
// Some GUI functions

#ifndef GUI_H
#define GUI_H

#include <def.h>

// Generalized font struct
struct font {
	char *chars;
	int height;
	int width;
	int char_size;
};

void gui_write(int x, int y, const u32 c[3], char *text);
void gui_init(char *font_path);

#endif

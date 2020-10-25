// MIT License, Copyright (c) 2020 Marvin Borner
// Mostly GFX function wrappers

#ifndef GUI_H
#define GUI_H

#include <def.h>
#include <gfx.h>
#include <list.h>

#define MAX_CHILDS 100

enum element_type { GUI_TYPE_CONTAINER, GUI_TYPE_BUTTON, GUI_TYPE_TEXTBOX };

struct element_button {
	const char *text;
	u32 color;
};

struct element_textbox {
	const char *text;
	u32 color;
};

struct element {
	enum element_type type;
	u32 window_id;
	struct context *ctx; // Coordinates are relative to container
	struct list *childs;
	void *data; // Who needs static types anyways :)
};

struct window {
	u32 id;
	const char *title;
	struct list *childs;
	struct context *ctx;
};

struct element *gui_init(const char *title, u32 width, u32 height);
struct element *gui_add_button(struct element *container, int x, int y, u32 width, u32 height,
			       const char *text, u32 color);

#endif

// MIT License, Copyright (c) 2020 Marvin Borner
// Mostly GFX function wrappers

#ifndef GUI_H
#define GUI_H

#include <def.h>
#include <gfx.h>
#include <list.h>

#define MAX_CHILDS 100

// TODO: Improve event types (maybe as struct header)
enum window_event_type { GUI_KEYBOARD = GFX_MAX + 1, GUI_MOUSE, GUI_MAX };
enum element_type { GUI_TYPE_CONTAINER, GUI_TYPE_BUTTON, GUI_TYPE_TEXTBOX };

enum container_flags { SPLIT };

struct element_container {
	u32 color_bg;
	enum container_flags flags;
};

struct element_button {
	char *text;
	u32 color_fg;
	u32 color_bg;
	enum font_type font_type;
	void (*on_click)();
};

struct element_textbox {
	const char *text;
	u32 color;
	enum font_type font_type;
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

struct gui_event_keyboard {
	char ch;
	int press;
	int scancode;
};

struct gui_event_mouse {
	int x;
	int y;
	int but1;
	int but2;
	int but3;
};

struct element *gui_init(const char *title, u32 width, u32 height);
void gui_event_loop(struct element *container);
struct element *gui_add_button(struct element *container, int x, int y, enum font_type font_type,
			       char *text, u32 color_bg, u32 color_fg);
struct element *gui_add_container(struct element *container, int x, int y, u32 width, u32 height,
				  u32 color_bg);

#endif

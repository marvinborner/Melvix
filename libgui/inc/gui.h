// MIT License, Copyright (c) 2020 Marvin Borner
// Mostly GFX function wrappers

#ifndef GUI_H
#define GUI_H

#include <def.h>
#include <gfx.h>
#include <list.h>

// TODO: Remove limits
#define MAX_CHILDS 100
#define MAX_INPUT_LENGTH 100

// TODO: Improve event types (maybe as struct header)
enum window_event_type { GUI_KILL, GUI_KEYBOARD, GUI_MOUSE, GUI_RESIZE, GUI_MAX };
enum element_type {
	GUI_TYPE_ROOT,
	GUI_TYPE_CONTAINER,
	GUI_TYPE_BUTTON,
	GUI_TYPE_LABEL,
	GUI_TYPE_TEXT_BOX,
	GUI_TYPE_TEXT_INPUT
};

enum container_flags { SPLIT };

struct element_event {
	void (*on_click)();
	void (*on_key)();
	void (*on_submit)();
};

struct element_container {
	int x;
	int y;
	u32 width;
	u32 height;
	u32 color_bg;
	enum container_flags flags;
};

struct element_button {
	int x;
	int y;
	char *text;
	u32 color_fg;
	u32 color_bg;
	enum font_type font_type;
};

struct element_label {
	int x;
	int y;
	char *text;
	u32 color_fg;
	u32 color_bg;
	enum font_type font_type;
};

struct element_text_box {
	int x;
	int y;
	char *text;
	u32 width;
	u32 height;
	u32 color_fg;
	u32 color_bg;
	enum font_type font_type;
};

struct element_text_input {
	int x;
	int y;
	u32 width;
	char text[MAX_INPUT_LENGTH];
	u32 color_fg;
	u32 color_bg;
	enum font_type font_type;
};

struct element {
	enum element_type type;
	u32 window_id;
	struct context *ctx; // Coordinates are relative to container
	struct element_event event;
	void *attributes;
	struct element *parent;
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

struct gui_event_resize {
	struct context *new_ctx;
};

struct element *gui_init(const char *title, u32 width, u32 height, u32 color_bg);
void gui_event_loop(struct element *container);
struct element *gui_add_button(struct element *container, int x, int y, enum font_type font_type,
			       const char *text, u32 color_bg, u32 color_fg);
struct element *gui_add_label(struct element *container, int x, int y, enum font_type font_type,
			      const char *text, u32 color_bg, u32 color_fg);
struct element *gui_add_text_box(struct element *container, int x, int y, u32 width, u32 height,
				 enum font_type font_type, const char *text, u32 color_bg,
				 u32 color_fg);
struct element *gui_add_text_input(struct element *container, int x, int y, u32 width,
				   enum font_type font_type, u32 color_bg, u32 color_fg);
struct element *gui_add_container(struct element *container, int x, int y, u32 width, u32 height,
				  u32 color_bg);
void gui_sync(struct element *elem);
void gui_remove_childs(struct element *elem);
void gui_remove_element(struct element *elem);

#endif

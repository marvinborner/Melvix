// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef GUI_H
#define GUI_H

#include <def.h>
#include <errno.h>
#include <libgui/gfx.h>
#include <vec.h>

enum gui_listener {
	GUI_LISTEN_MOUSEMOVE,
	GUI_LISTEN_MOUSECLICK,
};

enum gui_layer {
	GUI_LAYER_BG,
	GUI_LAYER_FG,
};

struct gui_event_mouse {
	u32 win;
	u32 widget;
	vec2 pos;
	s8 scroll; // Dir: -1 neg, +1 pos
	struct {
		u8 left : 1;
		u8 right : 1;
		u8 middle : 1;
	} but;
};

/**
 * Window operations
 */

void gui_new_custom_window(u32 *id, vec2 pos, vec2 size);
void gui_new_window(u32 *id);
void gui_redraw_window(u32 id);
void gui_redraw_window_only(u32 id); // Without widgets

/**
 * GFX wrappers
 */

void gui_clear(u32 win_id, u32 widget_id, enum gui_layer layer);
void gui_fill(u32 win_id, u32 widget_id, enum gui_layer layer, u32 c);
void gui_write(u32 win_id, u32 widget_id, enum gui_layer layer, vec2 pos, enum font_type font_type,
	       u32 c, const char *text);
void gui_load_image(u32 win_id, u32 widget_id, enum gui_layer layer, vec2 pos, vec2 size,
		    const char *path) NONNULL;
void gui_load_image_filter(u32 win_id, u32 widget_id, enum gui_layer layer, vec2 pos, vec2 size,
			   enum gfx_filter filter, const char *path) NONNULL;
void gui_draw_rectangle(u32 win_id, u32 widget_id, enum gui_layer layer, vec2 pos1, vec2 pos2,
			u32 c);
void gui_draw_border(u32 win_id, u32 widget_id, enum gui_layer layer, u32 width, u32 c);
void gui_draw_line(u32 win_id, u32 widget_id, enum gui_layer layer, vec2 pos1, vec2 pos2, u32 scale,
		   u32 c);

/**
 * Widget operations
 */

void gui_add_widget(u32 *widget, u32 win_id, u32 widget_id, vec2 pos, vec2 size);
void gui_new_widget(u32 *widget, u32 win_id, vec2 pos, vec2 size);
void gui_listen_widget(u32 win_id, u32 widget_id, enum gui_listener listener, u32 func);
void gui_redraw_widget(u32 win_id, u32 widget_id);

void gui_popup(const char *text);

vec2 gui_window_size(u32 win_id);

void gui_loop(void);

#endif

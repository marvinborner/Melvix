// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef GUI_H
#define GUI_H

#include <def.h>
#include <errno.h>
#include <libgui/gfx.h>
#include <vec.h>

enum gui_listener { GUI_LISTEN_MOUSEMOVE };

res gui_new_window(void);
res gui_redraw_window(u32 id);

res gui_fill(u32 win_id, u32 widget_id, u32 c);

res gui_add_widget(u32 win_id, u32 widget_id, vec2 size, vec2 pos);
res gui_new_widget(u32 win_id, vec2 size, vec2 pos);
res gui_listen_widget(u32 win_id, u32 widget_id, enum gui_listener listener, u32 func);
res gui_redraw_widget(u32 win_id, u32 widget_id);

vec2 gui_window_size(u32 win_id);

void gui_loop(void);

#endif

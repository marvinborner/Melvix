// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef WIDGETS_H
#define WIDGETS_H

#include <libgui/gfx.h>
#include <libgui/gui.h>

void gui_button_custom(u32 window, u32 widget, enum font_type font_type, u32 bg, u32 fg,
		       void (*click)(struct gui_event_mouse *event), const char *text);
void gui_button(u32 window, u32 widget, void (*click)(struct gui_event_mouse *event),
		const char *text);

#endif

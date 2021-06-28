// MIT License, Copyright (c) 2021 Marvin Borner

#include <def.h>
#include <libgui/gui.h>
#include <libgui/widgets.h>

#define TEXT_PAD 2

/**
 * Button
 */

void gui_button_custom(u32 window, u32 widget, enum font_type font_type, u32 bg, u32 fg,
		       void (*click)(struct gui_event_mouse *event), const char *text)
{
	vec2 font_size = gfx_font_size(font_type);
	vec2 size = vec2(font_size.x * strlen(text) + TEXT_PAD * 2, font_size.y + TEXT_PAD * 2);
	u32 button = gui_widget(window, widget, size);
	gui_fill(window, button, GUI_LAYER_BG, bg);
	gui_write(window, button, GUI_LAYER_FG, vec2(TEXT_PAD, TEXT_PAD), font_type, fg, text);
	gui_widget_listen(window, button, GUI_LISTEN_MOUSECLICK, (u32)click);
}

void gui_button(u32 window, u32 widget, void (*click)(struct gui_event_mouse *event),
		const char *text)
{
	gui_button_custom(window, widget, FONT_16, COLOR_WHITE, COLOR_BLACK, click, text);
}

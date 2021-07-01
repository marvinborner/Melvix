// MIT License, Copyright (c) 2021 Marvin Borner

#ifndef WIDGETS_H
#define WIDGETS_H

#include <libgui/gfx.h>
#include <libgui/gui.h>

/**
 * Button
 */

void gui_button_custom(u32 window, u32 widget, enum font_type font_type, u32 bg, u32 fg,
		       void (*click)(struct gui_event_mouse *event), const char *text);
void gui_button(u32 window, u32 widget, void (*click)(struct gui_event_mouse *event),
		const char *text);

/**
 * Plot
 */

struct gui_plot {
	f64 *data;
	u32 current;
	u32 count;
	struct gfx_context *ctx;
};

void gui_plot_iterate(struct gui_plot *plot);
void gui_plot_destroy(struct gui_plot *plot);
void gui_plot_draw(struct gui_plot *plot);
struct gui_plot *gui_plot_data(u32 window, u32 widget, const f64 *data, u32 count);
struct gui_plot *gui_plot_cubic(u32 window, u32 widget, f64 delta, s32 x_min, s32 x_max, f64 a,
				f64 b, f64 c, f64 d);

#endif

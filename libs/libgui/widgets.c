// MIT License, Copyright (c) 2021 Marvin Borner

#include <assert.h>
#include <def.h>
#include <libgui/gui.h>
#include <libgui/widgets.h>
#include <math.h>

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

/**
 * Plot
 */

static void gui_plot_redraw(struct gui_plot *plot)
{
	if (plot->current <= 1)
		return;

	assert(plot->current <= plot->count);

	gfx_fill(plot->ctx, COLOR_BG);

	f64 min = 0.0, max = 0.0;
	for (u32 i = 0; i < plot->current; i++) {
		f64 current = plot->data[i];
		if (current < min)
			min = current;
		if (current > max)
			max = current;
	}

	vec2 size = vec2_sub(plot->ctx->size, vec2(2, 2)); // Margin
	u32 diff = ABS(max - min);
	u32 step = size.x / (plot->current - 1);
	f64 scale = (f64)size.y / (diff + 1);

	for (u32 i = 1; i < plot->current; i++) {
		vec2 prev = vec2((i - 1) * step + 2, (max - plot->data[i - 1]) * scale);
		vec2 current = vec2(i * step + 2, (max - plot->data[i]) * scale);
		gfx_draw_line(plot->ctx, prev, current, 1, COLOR_RED);
	}
}

void gui_plot_iterate(struct gui_plot *plot)
{
	plot->current++;
	assert(plot->current <= plot->count);
	gui_plot_redraw(plot);
}

void gui_plot_destroy(struct gui_plot *plot)
{
	free(plot->data);
	free(plot);
}

void gui_plot_draw(struct gui_plot *plot)
{
	plot->current = plot->count;
	gui_plot_redraw(plot);
}

struct gui_plot *gui_plot_data(u32 window, u32 widget, const f64 *data, u32 count)
{
	f64 *buffer = malloc(count * sizeof(*data));
	memcpy(buffer, data, count * sizeof(*data));

	struct gui_plot *plot = malloc(sizeof(*plot));
	plot->data = buffer;
	plot->current = 0;
	plot->count = count;
	plot->ctx = gui_widget_context(window, widget, GUI_LAYER_FG);
	gui_fill(window, widget, GUI_LAYER_BG, COLOR_BG);
	return plot;
}

struct gui_plot *gui_plot_cubic(u32 window, u32 widget, f64 delta, s32 x_min, s32 x_max, f64 a,
				f64 b, f64 c, f64 d)
{
	assert(x_min < x_max);
	u32 diff = FABS(x_max - x_min);
	u32 count = mceil(diff * (1 / delta)) + 1;
	f64 *data = zalloc(count * sizeof(*data));

	for (u32 i = 0; i < count; i++)
		data[i] = mcubic(x_min + i * delta, a, b, c, d);

	struct gui_plot *plot = gui_plot_data(window, widget, data, count);

	free(data);
	return plot;
}

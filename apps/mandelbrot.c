// MIT License, Copyright (c) 2020 Marvin Borner

#include <conv.h>
#include <def.h>
#include <gui.h>
#include <input.h>
#include <print.h>
#include <random.h>
#include <str.h>
#include <sys.h>

void draw_pixel(struct window *win, int x, int y, u32 c)
{
	int pos = x * (win->bpp >> 3) + y * win->pitch;
	win->fb[pos + 0] = GET_BLUE(c);
	win->fb[pos + 1] = GET_GREEN(c);
	win->fb[pos + 2] = GET_RED(c);
	win->fb[pos + 3] = GET_ALPHA(c);
}

void draw_mandelbrot(struct window *win, int resolution)
{
	int height = win->height;
	int width = win->width;
	int max = resolution;

	for (int row = 0; row < height; row++) {
		for (int col = 0; col < width; col++) {
			double c_re = (col - width / 2.0) * 4.0 / width;
			double c_im = (row - height / 2.0) * 4.0 / width;
			double x = 0, y = 0;
			int iteration = 0;
			while (x * x + y * y <= 4 && iteration < max) {
				double x_new = x * x - y * y + c_re;
				y = 2 * x * y + c_im;
				x = x_new;
				iteration++;
			}
			srand(iteration);
			if (iteration < max)
				draw_pixel(win, col, row,
					   rand() << 16 | rand() << 8 | rand() | 0xff000000);
			else
				draw_pixel(win, col, row, 0xff000000);

			if (row % 50 == 0 && col == 0)
				gui_redraw();
		}
	}
	gui_redraw();
	print("Rendered mandelbrot successfully\n");
	yield();
}

int main()
{
	print("[mandelbrot window loaded]\n");

	struct window win = { 0 };
	win.width = 500;
	win.height = 300;
	gui_new_window(&win);
	gui_fill(&win, COLOR_BG);

	draw_mandelbrot(&win, 50);

	while (1) {
		yield();
	};

	return 0;
}

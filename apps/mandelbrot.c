// MIT License, Copyright (c) 2020 Marvin Borner

#include <conv.h>
#include <def.h>
#include <gfx.h>
#include <input.h>
#include <print.h>
#include <random.h>
#include <str.h>
#include <sys.h>

void draw_pixel(struct context *ctx, int x, int y, u32 c)
{
	int pos = x * (ctx->bpp >> 3) + y * ctx->pitch;
	ctx->fb[pos + 0] = GET_BLUE(c);
	ctx->fb[pos + 1] = GET_GREEN(c);
	ctx->fb[pos + 2] = GET_RED(c);
	ctx->fb[pos + 3] = GET_ALPHA(c);
}

void draw_mandelbrot(struct context *ctx, int resolution)
{
	int height = ctx->height;
	int width = ctx->width;
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
				draw_pixel(ctx, col, row,
					   rand() << 16 | rand() << 8 | rand() | 0xff000000);
			else
				draw_pixel(ctx, col, row, 0xff000000);

			if (row % 50 == 0 && col == 0)
				gfx_redraw();
		}
	}
	gfx_redraw();
	print("Rendered mandelbrot successfully\n");
	yield();
}

int main()
{
	/* print("[mandelbrot context loaded]\n"); */

	struct context ctx = { 0 };
	ctx.x = 500;
	ctx.y = 500;
	ctx.width = 500;
	ctx.height = 300;
	gfx_new_ctx(&ctx);
	gfx_fill(&ctx, COLOR_BG);

	draw_mandelbrot(&ctx, 50);

	while (1) {
		yield();
	};

	return 0;
}

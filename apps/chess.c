// MIT License, Copyright (c) 2021 Marvin Borner
// Ugly chess implementation to find the limits of libgui
// Asserts are generally not needed but don't hurt either

#include <assert.h>
#include <libgui/gui.h>
#include <print.h>

#define SIZE 8
#define TILE 36
#define WHITE_STARTS 1

#define DARK_COLOR 0xff946f51
#define LIGHT_COLOR 0xfff0d9b5

// Pieces
#define NONE 0
#define KING 1
#define PAWN 2
#define KNIGHT 3
#define BISHOP 5
#define ROOK 6
#define QUEEN 7
#define WHITE 8
#define BLACK 16

// Masks
#define TYPE_MASK 7
#define BLACK_MASK BLACK
#define WHITE_MASK WHITE
#define COLOR_MASK (WHITE_MASK | BLACK_MASK)

// Macros
#define COLOR(piece) (piece & COLOR_MASK)
#define IS_COLOR(piece, color) (COLOR(piece) == color)
#define TYPE(piece) (piece & TYPE_MASK)
#define IS_ROOK_OR_QUEEN(piece) ((piece & 6) == 6)
#define IS_BISHOP_OR_QUEEN(piece) ((piece & 5) == 5)
#define IS_SLIDING_PIECE(piece) ((piece & 4) != 0)

struct piece {
	u32 piece;
	u32 widget;
	const char *icon;
	struct {
		u8 moved : 1;
		// idk
	} bits;
};

typedef struct piece board[SIZE][SIZE];

static u32 win = 0; // Window
static board tiles = { 0 }; // Matrix

static void mouseclick(u32 widget_id, vec2 pos)
{
	UNUSED(pos);
	/* log("%d: %d %d\n", widget_id, pos.x, pos.y); */

	u32 x = widget_id / SIZE;
	u32 y = (widget_id % SIZE) - 1;
	u32 widget = tiles[x][y].widget;
	assert(gui_fill(win, widget, GUI_LAYER_BG, COLOR_MAGENTA) == EOK);
	gui_redraw_widget(win, widget);
}

static void create_board(void)
{
	u32 widget;
	for (u8 x = 0; x < 8; x++) {
		for (u8 y = 0; y < 8; y++) {
			widget = gui_new_widget(win, vec2(TILE, TILE), vec2(TILE * x, TILE * y));
			assert(widget > 0);

			u8 colored = (x + y + 1) % 2 == 0;
			u8 colored_piece = y < SIZE / 2;
#if !WHITE_STARTS
			colored = !colored;
			colored_piece = !colored_piece;
#endif

			struct piece *tile = &tiles[x][y];
			tile->piece |= colored_piece ? BLACK : WHITE;
			tile->widget = widget;
			tile->icon = "/icons/chess-king-36.png";

			assert(gui_fill(win, widget, GUI_LAYER_BG,
					colored ? DARK_COLOR : LIGHT_COLOR) == EOK);

			enum gfx_filter filter =
				colored_piece ? GFX_FILTER_NONE : GFX_FILTER_INVERT;
			assert(gui_load_image_filter(win, widget, GUI_LAYER_FG, vec2(0, 0),
						     vec2(TILE, TILE), filter, tile->icon) == EOK);

			assert(gui_listen_widget(win, widget, GUI_LISTEN_MOUSECLICK,
						 (u32)mouseclick) == EOK);
		}
	}

	assert(gui_redraw_window(win) == EOK);
}

int main(void)
{
	assert((win = gui_new_window()) > 0);
	create_board();
	gui_loop();
	return 0;
}

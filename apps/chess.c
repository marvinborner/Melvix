// MIT License, Copyright (c) 2021 Marvin Borner
// Ugly chess implementation to find the limits of libgui
// Asserts are generally not needed but don't hurt either

#include <assert.h>
#include <libgui/gui.h>
#include <print.h>

// Config
#define SIZE 8
#define TILE 48
#define WHITE_STARTS 1
#define DARK_COLOR 0xff946f51
#define LIGHT_COLOR 0xfff0d9b5
#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

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
	char name[8];
	struct {
		u8 moved : 1;
		// idk
	} bits;
};

typedef struct piece board[SIZE][SIZE];

static u32 win = 0; // Window
static board tiles = { 0 }; // Matrix
static vec2 selected = { -1, -1 }; // Selected tile

static void load_image(struct piece *tile)
{
	char icon[48] = { 0 };
	snprintf(icon, sizeof(icon), "/icons/chess-%s-%d.png", tile->name, TILE);
	enum gfx_filter filter = IS_COLOR(tile->piece, BLACK) ? GFX_FILTER_NONE : GFX_FILTER_INVERT;

	/* assert(gui_fill(win, tile->widget, GUI_LAYER_FG, 0) == EOK); */
	assert(gui_load_image_filter(win, tile->widget, GUI_LAYER_FG, vec2(0, 0), vec2(TILE, TILE),
				     filter, icon) == EOK);
}

static void mouseclick(u32 widget_id, vec2 pos)
{
	UNUSED(pos);

	vec2 clicked = vec2(0, 0);
	for (u32 x = 0; x < SIZE; x++)
		for (u32 y = 0; y < SIZE; y++)
			if (tiles[x][y].widget == widget_id)
				clicked = vec2(x, y);

	struct piece *clicked_piece = &tiles[clicked.x][clicked.y];

	if (vec2_eq(clicked, selected))
		return;

	if (selected.x != (u32)-1) {
		struct piece *selected_piece = &tiles[selected.x][selected.y];

		clicked_piece->piece = selected_piece->piece;
		selected_piece->piece = 0;

		strlcpy(clicked_piece->name, selected_piece->name, sizeof(clicked_piece->name));
		selected_piece->name[0] = '\0';

		/* assert(gui_fill(win, selected_piece->widget, GUI_LAYER_FG, 0) == EOK); */
		load_image(clicked_piece);

		assert(gui_redraw_window(win) == EOK);

		selected = vec2(-1, -1);
	} else if (clicked_piece->piece) {
		assert(gui_redraw_widget(win, clicked_piece->widget) == EOK);
		selected = clicked;
	}
}

static const char *resolve_name(u32 piece, char buf[8])
{
	const char *name = NULL;
	switch (piece & TYPE_MASK) {
	case KING:
		name = "king";
		break;
	case PAWN:
		name = "pawn";
		break;
	case KNIGHT:
		name = "knight";
		break;
	case BISHOP:
		name = "bishop";
		break;
	case ROOK:
		name = "rook";
		break;
	case QUEEN:
		name = "queen";
		break;
	default:
		err(1, "Unknown piece %d\n", piece);
	}

	strlcpy(buf, name, 8);

	return buf;
}

static u32 fen_resolve_letter(char ch)
{
	u32 piece = 0;

	switch (ch) {
	case 'k':
		piece = KING | BLACK;
		break;
	case 'K':
		piece = KING | WHITE;
		break;
	case 'p':
		piece = PAWN | BLACK;
		break;
	case 'P':
		piece = PAWN | WHITE;
		break;
	case 'n':
		piece = KNIGHT | BLACK;
		break;
	case 'N':
		piece = KNIGHT | WHITE;
		break;
	case 'b':
		piece = BISHOP | BLACK;
		break;
	case 'B':
		piece = BISHOP | WHITE;
		break;
	case 'r':
		piece = ROOK | BLACK;
		break;
	case 'R':
		piece = ROOK | WHITE;
		break;
	case 'q':
		piece = QUEEN | BLACK;
		break;
	case 'Q':
		piece = QUEEN | WHITE;
		break;
	default:
		err(1, "Invalid letter (%c)!\n", ch);
	}

	return piece;
}

// TODO: Add more than basic fen support
static void fen_parse(const char *fen)
{
	if (!fen || !*fen)
		return;

	u8 x = 0, y = 0;
	for (const char *p = fen; p && *p; p++) {
		if (*p == ' ')
			break;

		if (*p == '/') {
			x = 0;
			y++;
			continue;
		}

		if (*p >= '0' && *p <= '9')
			continue;

		u32 piece = fen_resolve_letter(*p);

		tiles[x][y].piece = piece;
		resolve_name(piece, tiles[x][y].name);

		x++;
	}
}

static void draw_board(void)
{
	for (u8 x = 0; x < 8; x++) {
		for (u8 y = 0; y < 8; y++) {
			u32 widget =
				gui_new_widget(win, vec2(TILE, TILE), vec2(TILE * x, TILE * y));
			assert((signed)widget > 0);

			u8 colored = (x + y + 1) % 2 == 0;
#if !WHITE_STARTS
			colored = !colored;
#endif
			assert(gui_fill(win, widget, GUI_LAYER_BG,
					colored ? DARK_COLOR : LIGHT_COLOR) == EOK);

			struct piece *tile = &tiles[x][y];
			assert(gui_listen_widget(win, widget, GUI_LISTEN_MOUSECLICK,
						 (u32)mouseclick) == EOK);

			tile->widget = widget;

			if (tile->piece)
				load_image(tile);
		}
	}

	assert(gui_redraw_window(win) == EOK);
}

int main(void)
{
	assert((win = gui_new_window()) > 0);
	fen_parse(START_FEN);
	draw_board();

	gui_loop();
	return 0;
}

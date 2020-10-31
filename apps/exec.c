#include <gui.h>
#include <print.h>
#include <sys.h>

#define HEIGHT 32
#define WIDTH 300
#define BORDER 2

void on_submit(struct gui_event_keyboard *event, struct element *elem)
{
	(void)event;
	char *inp = ((struct element_text_input *)elem->data)->text;
	exec(inp, inp, NULL);
}

int main()
{
	struct element *root =
		gui_init("Exec", WIDTH + BORDER * 2, HEIGHT + BORDER * 2, COLOR_BLACK);
	struct element *input =
		gui_add_text_input(root, BORDER, BORDER, WIDTH, FONT_32, COLOR_WHITE, COLOR_BLACK);

	input->event.on_submit = on_submit;

	gfx_redraw_focused(); // TODO: Remove once partial redrawing is finished
	gui_event_loop(root);

	return 0;
}

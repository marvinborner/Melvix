// MIT License, Copyright (c) 2020 Marvin Borner

#include <gui.h>
#include <mem.h>
#include <print.h>
#include <str.h>
#include <sys.h>

#define PATH "/bin/"

#define HEIGHT 32
#define WIDTH 300
#define BORDER 2

void on_submit(struct gui_event_keyboard *event, struct element *elem)
{
	(void)event;
	char *inp = ((struct element_text_input *)elem->data)->text;

	// TODO: Support more than one arg
	char *inp_copy = strdup(inp);
	char *space = inp_copy;
	char *arg = NULL;
	if ((space = strchr(space, ' '))) {
		inp[space - inp_copy] = '\0';
		space++;
		arg = space;
	}
	free(inp_copy);

	u8 l = strlen(PATH) + strlen(inp) + 1;
	char *final = malloc(l);
	strcat(final, PATH);
	strcat(final, inp);
	exec(final, inp, arg, NULL);
}

int main()
{
	struct element *root =
		gui_init("Exec", WIDTH + BORDER * 2, HEIGHT + BORDER * 2, COLOR_BLACK);
	struct element *input =
		gui_add_text_input(root, BORDER, BORDER, WIDTH, FONT_32, COLOR_WHITE, COLOR_BLACK);

	input->event.on_submit = on_submit;

	gui_event_loop(root);

	return 0;
}

// MIT License, Copyright (c) 2020 Marvin Borner
// TODO: Fix green memory artifacts

#include <gui.h>
#include <mem.h>
#include <print.h>
#include <str.h>
#include <sys.h>

static struct element *root = NULL;

struct dirent {
	u32 inode_num;
	u16 total_len;
	u8 name_len;
	u8 type_indicator;
	char name[];
};

void render_list(const char *path);
void on_click(struct event_mouse *event, struct element *elem)
{
	(void)event;
	char *value = ((struct element_label *)elem->data)->text;
	u8 l = strlen(elem->attributes) + strlen(value) + 2;
	char *full = malloc(l);
	strcat(full, elem->attributes);
	full[strlen(elem->attributes)] = '/';
	strcat(full, value);
	render_list(full);
}

// TODO: Dir iterator as kernel syscall?
void render_list(const char *path)
{
	static struct element *list = NULL;
	if (list)
		gui_remove_element(list);
	list = gui_add_container(root, 0, 0, 600, 400, COLOR_BLACK);

	struct dirent *d = read(path);

	int sum = 0;
	int calc = 0;
	int cnt = 0;
	do {
		calc = (sizeof(struct dirent) + d->name_len + 4) & ~0x3;
		sum += d->total_len;
		d->name[d->name_len] = '\0';
		struct element *label = gui_add_label(list, 5, cnt * (gfx_font_height(FONT_16) + 5),
						      FONT_16, d->name, COLOR_BLACK, COLOR_WHITE);
		label->attributes = (char *)path;

		if (d->type_indicator == 2) // Dir
			label->event.on_click = on_click;

		if (d->total_len != calc && sum == 1024)
			d->total_len = calc;
		d = (struct dirent *)((u32)d + d->total_len);
		cnt++;
	} while (sum < 1024); // TODO: Remove magic constants
}

int main()
{
	root = gui_init("Files", 600, 400, COLOR_BLACK);

	render_list("/.");
	gfx_redraw_focused(); // TODO: Remove

	gui_event_loop(root);

	return 0;
}

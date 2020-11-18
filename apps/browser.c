// MIT License, Copyright (c) 2020 Marvin Borner

#include <conv.h>
#include <def.h>
#include <gfx.h>
#include <gui.h>
#include <input.h>
#include <mem.h>
#include <net.h>
#include <print.h>
#include <str.h>

static struct element *root;
static struct element *code_label;
static struct element *output;

// Temporary: Will be moved to libnet
char **dns_split(char *url, char **buf)
{
	strchr(url, '.')[0] = '\0';
	char *first = url;
	char *second = url + strlen(url) + 1;
	buf[0] = first;
	buf[1] = second;

	return buf;
}

void on_submit(void *event, struct element *box)
{
	(void)event;
	char *url = ((struct element_text_input *)box->data)->text;

	char *path = strchr(url, '/');
	if (path) {
		path[0] = '\0';
		path++;
	}
	char *query = http_query_get(url, path ? path : "/");

	char *dns[2];
	dns_split(url, dns);
	struct element_text_box *l = output->data;
	struct element_label *c = code_label->data;

	struct socket *socket = net_open(S_TCP);
	if (socket && net_connect(socket, dns_request(dns[0], dns[1]), 80)) {
		net_send(socket, query, strlen(query));
		char buf[4096] = { 0 };
		net_receive(socket, buf, 4096);
		l->text = http_data(buf);
		c->text = http_code(buf);
	} else {
		l->text = strdup("Can't connect to server.");
		c->text = strdup("000");
	}
	gui_sync(root, output);
	gui_sync(root, code_label);
	/* net_close(socket); */ // TODO: Fix net close before FIN/ACK got ACK'ed
}

int main()
{
	// TODO: Dynamic element positioning
	root = gui_init("browser", 640, 400, COLOR_BG);
	code_label = gui_add_label(root, 2, 2, FONT_24, "000", COLOR_WHITE, COLOR_BLACK);
	struct element *text_input =
		gui_add_text_input(root, 40, 2, 598, FONT_24, COLOR_WHITE, COLOR_BLACK);
	output = gui_add_text_box(root, 2, 28, 636, 370, FONT_16, "Enter URL and press Enter :)",
				  COLOR_WHITE, COLOR_BLACK);

	text_input->event.on_submit = on_submit;

	gui_event_loop(root);

	return 0;
}

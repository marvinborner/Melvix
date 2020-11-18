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
static struct element *output;

char *http_get(char *url)
{
	char *query = malloc(27 + strlen(url)); // TODO: Dynamic http length etc
	query[0] = '\0';
	strcat(query, "GET / HTTP/1.1\r\nHost: ");
	strcat(query, url);
	strcat(query, "\r\n\r\n");
	return query;
}

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
	char *query = http_get(url);
	char *dns[2];
	dns_split(url, dns);
	struct element_text_box *l = output->data;

	struct socket *socket = net_open(S_TCP);
	if (socket && net_connect(socket, dns_request(dns[0], dns[1]), 80)) {
		net_send(socket, query, strlen(query));
		u8 buf[4096] = { 0 };
		net_receive(socket, buf, 4096);
		l->text = (char *)buf;
		gui_sync(root, output);
	} else {
		print("Couldn't connect!\n");
	}
	/* net_close(socket); */ // TODO: Fix net close before FIN/ACK got ACK'ed
}

int main()
{
	root = gui_init("browser", 600, 400, COLOR_BG);
	struct element *text_input =
		gui_add_text_input(root, 10, 10, 580, FONT_24, COLOR_WHITE, COLOR_BLACK);
	output = gui_add_text_box(root, 10, 50, 580, 340, FONT_24, "Enter URL and press Enter :)",
				  COLOR_WHITE, COLOR_BLACK);

	text_input->event.on_submit = on_submit;

	gui_event_loop(root);

	return 0;
}

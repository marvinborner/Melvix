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
#include <xml.h>

#define WIDTH 640
#define HEIGHT 400
#define FONT_HEIGHT 24
#define LABEL_WIDTH 36 // Thx Lars

static struct element *root;
static struct element *code_label;
static struct element *output;

u32 status_color(char *http_code)
{
	u32 c = 0;
	switch (http_code[0]) {
	case '1': // Information response
		c = COLOR_BLUE;
		break;
	case '2': // Successful response
		c = COLOR_GREEN;
		break;
	case '3': // Redirects
		c = COLOR_YELLOW;
		break;
	case '4': // Client error
		c = COLOR_RED;
		break;
	case '5': // Server error
		c = COLOR_MAGENTA;
		break;
	default:
		c = COLOR_WHITE;
		break;
	}
	return c;
}

void parse(void *data, u32 len, char **out)
{
	struct xml_token tokens[128];

	struct xml parser;
	xml_init(&parser);
	void *buffer = data;
	len = strlen(data);
	enum xml_error err = xml_parse(&parser, buffer, len, tokens, 128);
	printf("%s\n", data);
	if (err != XML_SUCCESS) {
		printf("ERROR %d\n", err);
		return;
	}

	*out[0] = '\0';

	u32 pos = 0;
	u32 indent = 0;
	char name[16] = { 0 };
	for (u32 i = 0; i < parser.ntokens; i++) {
		const struct xml_token *token = tokens + i;
		name[0] = '\0';
		switch (token->type) {
		case XML_START_TAG:
			for (u32 j = 0; j < indent; j++)
				strcat(*out, "\t");
			indent++;
			memcpy(&name, (u8 *)buffer + token->start_pos,
			       token->end_pos - token->start_pos);
			name[token->end_pos - token->start_pos] = '\0';
			printf("%s\n", name);
			print("START TAG\n");
			strcat(*out, name);
			strcat(*out, "\n");
			break;
		case XML_END_TAG:
			indent--;
			for (u32 j = 0; j < indent; j++)
				strcat(*out, "\t");
			memcpy(&name, (u8 *)buffer + token->start_pos,
			       token->end_pos - token->start_pos);
			name[token->end_pos - token->start_pos] = '\0';
			print("END TAG\n");
			strcat(*out, name);
			strcat(*out, "/\n");
			break;
		default:
			break;
		}

		i += token->size;
	}
	printf("%s\n", *out);
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

	u32 ip = 0;
	if (!ip_pton(url, &ip)) {
		ip = dns_request(url);
	}

	struct element_text_box *l = output->data;
	struct element_label *c = code_label->data;

	struct socket *socket = net_open(S_TCP);
	if (socket && net_connect(socket, ip, 8000)) {
		net_send(socket, query, strlen(query));
		char buf[4096] = { 0 };
		char parsed[4096] = { 0 };
		net_receive(socket, buf, 4096);
		parse(http_data(buf), 4096, (char **)&parsed);
		l->text = http_data(parsed);
		c->text = http_code(buf);
		c->color_fg = status_color(c->text);
	} else {
		l->text = strdup("Can't connect to server.");
		c->text = strdup("000");
		c->color_fg = COLOR_RED;
	}
	gui_sync(root, output);
	gui_sync(root, code_label);
	net_close(socket);
}

int main()
{
	// TODO: Dynamic element positioning
	root = gui_init("browser", WIDTH, HEIGHT, COLOR_BG);
	code_label = gui_add_label(root, 0, 0, FONT_24, "000", COLOR_BLACK, COLOR_WHITE);
	struct element *text_input =
		gui_add_text_input(root, LABEL_WIDTH, 0, 100, FONT_24, COLOR_WHITE, COLOR_BLACK);
	memcpy(((struct element_text_input *)text_input->data)->text, strdup("127.0.0.1"), 10);
	output = gui_add_text_box(root, 0, FONT_HEIGHT + 2, 100, 100, FONT_16,
				  "Enter URL and press Enter :)", COLOR_WHITE, COLOR_BLACK);
	gui_sync(root, text_input);

	text_input->event.on_submit = on_submit;

	gui_event_loop(root);

	return 0;
}

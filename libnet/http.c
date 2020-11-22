// MIT License, Copyright (c) 2020 Marvin Borner

#include <assert.h>
#include <conv.h>
#include <def.h>
#include <http.h>
#include <mem.h>
#include <print.h>
#include <str.h>

char *http_data(char *r)
{
	char *h = NULL;
	for (u32 i = 0; i < strlen(r); ++i) {
		if (r[i] == '\r' && r[i + 1] == '\n' && r[i + 2] == '\r' && r[i + 3] == '\n') {
			h = &r[i + 4];
			break;
		}
	}
	return h;
}

char *http_code(char *r)
{
	char *code = malloc(4);
	char tmp = r[12];
	r[12] = '\0';
	memcpy(code, r + 9, 3);
	code[3] = '\0';
	r[12] = tmp;
	return code;
}

u32 http_response(const char *http_code, u32 content_length, const char *data, char *resp)
{
	char buf[16] = { 0 };

	resp[0] = '\0';
	strcat(resp, "HTTP/1.1 ");
	strcat(resp, buf);
	strcat(resp, http_code);
	strcat(resp, "\r\n");
	strcat(resp, "Content-Length: ");
	strcat(resp, conv_base(content_length, buf, 10, 0));
	strcat(resp, "\r\n");
	strcat(resp, "Server: Melvix\r\n");
	strcat(resp, "Content-Type: text/html\r\n");
	strcat(resp, "Connection: close\r\n\r\n");
	u32 len = strlen(resp);
	memcpy(&resp[len], data, content_length);

	return len + content_length;
}

char *http_query_get(const char *url, const char *path)
{
	char *query = malloc(27 + strlen(url)); // TODO: Dynamic http length etc
	query[0] = '\0';
	strcat(query, "GET ");
	if (path[0] != '/')
		strcat(query, "/");
	strcat(query, path);
	strcat(query, " HTTP/1.1\r\nHost: ");
	strcat(query, url);
	strcat(query, "\r\n\r\n");
	return query;
}

char *http_query_path(const char *query, char *path)
{
	u8 b = 0;
	u32 s = 0;
	u32 e = 0;

	while (1) {
		if (!b && query[e] == ' ' && query[++e]) {
			s = e;
			b = 1;
		} else if (b && query[e] == ' ') {
			strncat(path, &query[s], e - s);
			break;
		} else if (query[e] == '\0') {
			return NULL;
		}
		e++;
	}

	return path;
}

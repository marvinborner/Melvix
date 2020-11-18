// MIT License, Copyright (c) 2020 Marvin Borner

#include <def.h>
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

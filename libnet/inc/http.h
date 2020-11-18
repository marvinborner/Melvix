// MIT License, Copyright (c) 2020 Marvin Borner

#ifndef HTTP_H
#define HTTP_H

char *http_data(char *response);
char *http_query_get(const char *url, const char *path);
char *http_code(char *r);

#endif
